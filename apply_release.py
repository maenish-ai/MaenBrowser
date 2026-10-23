#!/usr/bin/env python3
"""Apply the 1.9.7 release/update + popup fix to a clean MaenBrowser 1.9.6 checkout.

Does NOT alter the input repository, publish a release, or install software.
Exports changed-file upload ZIP and full-source ZIP with SHA256 manifest.
"""
from __future__ import annotations
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tempfile
import zipfile

HERE = Path(__file__).resolve().parent
FILES = HERE / 'files'
NEW_VERSION = '1.9.7'
REPOSITORY = 'https://github.com/maenish-ai/MaenBrowser.git'


def replace_once(path: Path, before: str, after: str):
    old = path.read_text(encoding='utf-8')
    n = old.count(before)
    if n != 1:
        raise RuntimeError(f'Unexpected source at {path}: expected exactly one match, found {n}: {before[:110]!r}')
    path.write_text(old.replace(before, after, 1), encoding='utf-8', newline='\n')


def patch(root: Path):
    if (root / 'VERSION').read_text(encoding='utf-8').strip() != '1.9.6':
        raise RuntimeError('Expected a clean MaenBrowser 1.9.6 checkout. Aborting without changes to source.')
    # Native build metadata / installer: keep matching versions everywhere.
    for name in ['VERSION', 'CMakeLists.txt', 'include/maenbrowser/version.h',
                 'installer/MaenBrowser.nsi', 'src/updater/update_manager.cpp']:
        path = root / name
        text = path.read_text(encoding='utf-8')
        if '1.9.6' not in text:
            raise RuntimeError(f'Expected version anchor not present: {name}')
        path.write_text(text.replace('1.9.6', NEW_VERSION), encoding='utf-8', newline='\n')

    # The packaged extension is always bundled with the exe. Its worker checks
    # signed-HTTPS GitHub metadata and only offers a manual official download.
    manifest_path = root / 'assets/companion/manifest.json'
    manifest = json.loads(manifest_path.read_text(encoding='utf-8'))
    if manifest['version'] != '1.9.6' or manifest['background']['service_worker'] != 'background-1.9.6.js':
        raise RuntimeError('Unexpected extension version/worker: cannot safely patch')
    manifest['version'] = NEW_VERSION
    manifest['background']['service_worker'] = 'background-1.9.7.js'
    manifest['permissions'] = list(dict.fromkeys([*manifest['permissions'], 'notifications']))
    policy = manifest['content_security_policy']['extension_pages']
    if 'https://api.github.com' not in policy:
        policy = policy.replace('connect-src ', 'connect-src https://api.github.com ', 1)
    manifest['content_security_policy']['extension_pages'] = policy
    manifest_path.write_text(json.dumps(manifest, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')

    # Route ordinary HTTP links back to the initiating tab. Authentication and
    # payment popups must remain native Chromium popups, retaining their opener.
    client = root / 'src/app/maen_client.cpp'
    replace_once(client, '#include "src/app/external_protocol.h"',
                 '#include "src/app/external_protocol.h"\n#include "src/app/popup_policy.h"')
    replace_once(client,
        '  // Route only explicit, same-origin HTTP(S) link clicks into the existing\n'
        '  // tab. Cross-origin popups may be required for sign-in or payment flows.\n'
        '  // Never navigate on script-generated popups without a user gesture.',
        '  // Keep OAuth/payment flows as real popups: they need their opener and\n'
        '  // same Chromium context. Route only ordinary user-clicked links into\n'
        '  // the existing tab, including safe cross-origin links.')
    replace_once(client,
        '  if (!user_gesture || !source.valid || !destination.valid ||\n'
        '      (destination.scheme != "https" && destination.scheme != "http") ||\n'
        '      source.scheme != destination.scheme || source.host != destination.host ||\n'
        '      protection::BlockNavigation(target_url.ToString())) {',
        '  if (!user_gesture || !source.valid || !destination.valid ||\n'
        '      (destination.scheme != "https" && destination.scheme != "http") ||\n'
        '      protection::BlockNavigation(target_url.ToString()) ||\n'
        '      ShouldPreservePopup(source.host, destination.host,\n'
        '                          browser->GetMainFrame()->GetURL().ToString(),\n'
        '                          target_url.ToString())) {')

    preflight = root / 'tools/preflight.ps1'
    replace_once(preflight,
        '# guarded same-origin navigation; false must remain available for auth/payment.',
        '# guarded user-click navigation; false must remain available for auth/payment.')
    replace_once(preflight,
        '    -not $popupBody.Contains("source.host != destination.host") -or\n'
        '    -not $popupBody.Contains("source.scheme != destination.scheme") -or\n'
        '    -not $popupBody.Contains("protection::BlockNavigation") -or',
        '    -not $popupBody.Contains("ShouldPreservePopup") -or\n'
        '    -not $popupBody.Contains("protection::BlockNavigation") -or')
    replace_once(preflight,
        'Popup policy must preserve OAuth/payment/login popups and restrict same-origin redirects.',
        'Popup policy must preserve OAuth/payment/login popups and restrict auto-redirects.')

    # Desktop-app OAuth loopback callbacks must remain HTTP when HTTPS-only is
    # enabled: local callback servers rarely offer a trusted HTTPS certificate.
    protection = root / 'src/protection/protection.cpp'
    replace_once(protection,
        '  if (s->https_only && u.scheme == "http") target.replace(0, 4, "https");',
        '  const bool loopback = u.host == "localhost" || u.host == "127.0.0.1" || u.host == "::1" || u.host == "[::1]";\n'
        '  if (s->https_only && u.scheme == "http" && !loopback) target.replace(0, 4, "https");')

    background = root / 'assets/companion/background.js'
    replace_once(background, "import {idlePolicy} from './memory-policy.js';",
        "import {idlePolicy} from './memory-policy.js';\n"
        "import {checkForRelease,RELEASES_PAGE,UPDATE_ALARM,UPDATE_NOTIFICATION_ID} from './release-updates.js';\n"
        "import {shouldPreserveAuthPopup} from './popup-policy.js';")
    replace_once(background,
        "  await chrome.alarms.create('protection-state-sync',{periodInMinutes:15});",
        "  await chrome.alarms.create('protection-state-sync',{periodInMinutes:15});\n"
        "  await chrome.alarms.create(UPDATE_ALARM,{periodInMinutes:360});\n"
        "  // Never block startup on a network request, and never install silently.\n"
        "  void checkForRelease().catch(()=>{});")
    replace_once(background,
        "  if(alarm.name==='protection-state-sync'){await syncProtectionBadge();return;}",
        "  if(alarm.name==='protection-state-sync'){await syncProtectionBadge();return;}\n"
        "  if(alarm.name===UPDATE_ALARM){await checkForRelease().catch(()=>{});return;}")
    replace_once(background,
        "const sensitivePopupHosts=['accounts.google.com','login.microsoftonline.com','login.live.com',\n"
        " 'appleid.apple.com','paypal.com','www.paypal.com','checkout.stripe.com'];",
        "const sensitivePopupHosts=['accounts.google.com','login.microsoftonline.com','login.live.com',\n"
        " 'appleid.apple.com','paypal.com','www.paypal.com','checkout.stripe.com','claude.ai','anthropic.com'];")
    replace_once(background,
        "  const host=new URL(url).hostname.toLowerCase();\n"
        "  if(sensitivePopupHosts.some(h=>host===h||host.endsWith('.'+h)))return;\n"
        "  const [source,target]=await Promise.all([chrome.tabs.get(sourceTabId),chrome.tabs.get(tabId)]);",
        "  const host=new URL(url).hostname.toLowerCase();\n"
        "  const [source,target]=await Promise.all([chrome.tabs.get(sourceTabId),chrome.tabs.get(tabId)]);\n"
        "  if(sensitivePopupHosts.some(h=>host===h||host.endsWith('.'+h)) ||\n"
        "     shouldPreserveAuthPopup(url,source?.url||''))return;")
    replace_once(background,
        "  if(targetWindow.type!=='popup'&&siblings.length!==1)return;",
        "  // Moving true popup windows destroys window.opener in many OAuth flows.\n"
        "  if(targetWindow.type==='popup'||siblings.length!==1)return;")
    # Small UX: notification opens the official release inside MaenBrowser.
    with background.open('a', encoding='utf-8', newline='\n') as f:
        f.write("\nchrome.notifications?.onClicked?.addListener(id=>{\n"
                "  if(id===UPDATE_NOTIFICATION_ID)chrome.tabs.create({url:RELEASES_PAGE}).catch(()=>{});\n"
                "});\n")

    # Test and CMake wiring.
    cmake = root / 'CMakeLists.txt'
    replace_once(cmake, '  src/app/maen_client.h\n',
                 '  src/app/maen_client.h\n  src/app/popup_policy.h\n')
    replace_once(cmake,
        'add_test(NAME resource_policy COMMAND MaenResourceTests)',
        'add_test(NAME resource_policy COMMAND MaenResourceTests)\n\n'
        'add_executable(MaenPopupPolicyTests tests/popup_policy_test.cpp)\n'
        'target_include_directories(MaenPopupPolicyTests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")\n'
        'add_test(NAME popup_policy COMMAND MaenPopupPolicyTests)')
    # Native tests must not be shipped in the browser runtime payload.
    ci = root / '.github/workflows/maenbrowser-ci.yml'
    replace_once(ci, '    branches: ["main", "development/**"]\n  workflow_dispatch:',
                 '    branches: ["main", "development/**"]\n    tags: ["v*"]\n  workflow_dispatch:')
    replace_once(ci,
        '          Remove-Item "dist\\MaenResourceTests.exe" -ErrorAction SilentlyContinue',
        '          Remove-Item "dist\\MaenResourceTests.exe" -ErrorAction SilentlyContinue\n'
        '          Remove-Item "dist\\MaenPopupPolicyTests.exe" -ErrorAction SilentlyContinue')
    replace_once(ci,
        '          Get-ChildItem "dist\\companion" -Filter "background-*.js" | Where-Object { $_.Name -ne "background-1.9.6.js" } | Remove-Item -Force',
        '          Get-ChildItem "dist\\companion" -Filter "background-*.js" | Where-Object { $_.Name -ne "background-1.9.7.js" } | Remove-Item -Force')
    release_job = '''\n  publish-release:
    name: Publish verified Windows GitHub Release
    if: startsWith(github.ref, 'refs/tags/v')
    needs: [configuration, build-setup]
    runs-on: ubuntu-latest
    permissions:
      contents: write
    steps:
      - name: Refuse publishing unverified preview runtime
        env:
          MEDIA_READY: ${{ needs.configuration.outputs.media_ready }}
        shell: bash
        run: |
          if [[ "$MEDIA_READY" != "true" ]]; then
            echo "::error::A public release requires the verified media-enabled CEF archive secrets. Stock CEF is preview only."
            exit 1
          fi
      - uses: actions/download-artifact@v4
        with:
          name: MaenBrowser-Windows-x64-Setup-H264
          path: release-files
      - name: Publish tagged Setup as GitHub Release
        env:
          GH_TOKEN: ${{ github.token }}
        shell: bash
        run: |
          set -euo pipefail
          tag="$GITHUB_REF_NAME"
          if [[ ! "$tag" =~ ^v[0-9]+\\.[0-9]+\\.[0-9]+$ ]]; then
            echo "::error::Release tag must be vMAJOR.MINOR.PATCH"
            exit 1
          fi
          version="${tag#v}"
          setup="release-files/MaenBrowser-${version}-Setup.exe"
          test -s "$setup" || { echo "::error::Version-matched Setup missing"; exit 1; }
          gh release create "$tag" "$setup" --verify-tag --latest \\
            --title "MaenBrowser ${version}" --generate-notes
'''
    with ci.open('a', encoding='utf-8', newline='\n') as f:
        f.write(release_job)

    # Release summary and quickstart reference.
    readme = root / 'README.md'
    replace_once(readme, '# MaenBrowser 1.9.6 —', '# MaenBrowser 1.9.7 —')
    replace_once(readme, '- [Arabic upload/build guide](START_HERE_AR.md)',
        '- [Arabic upload/build guide](START_HERE_AR.md)\n'
        '- [Update notifications and safe sign-in routing](docs/RELEASE_1.9.7_AR.md)')
    replace_once(readme, '## Changes',
        '## 1.9.7 release highlights\n'
        '- Checks official stable GitHub Releases at startup and every six hours; '
        'notifies once per version and opens the official page on click. '
        'Installer is never downloaded or executed automatically.\n'
        '- Routes ordinary user-clicked cross-site new-window links into the same tab, '
        'but preserves authentication/payment windows and their opener.\n'
        '- Keeps localhost loopback OAuth callbacks on HTTP when HTTPS-only mode is enabled.\n'
        '- Tag `v1.9.7` publishes the verified Windows Setup only after the build/tests pass; '
        'stock CEF previews are not public releases.\n\n## Changes')
    for path in FILES.rglob('*'):
        if path.is_file():
            dest = root / path.relative_to(FILES)
            if dest.exists():
                raise RuntimeError(f'New-file collision: {dest}')
            dest.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(path, dest)
    (root / 'assets/companion/background-1.9.7.js').write_text("import './background.js';\n", encoding='utf-8')
    changed = [
        'VERSION', 'CMakeLists.txt', 'include/maenbrowser/version.h',
        'installer/MaenBrowser.nsi', 'src/updater/update_manager.cpp',
        'assets/companion/manifest.json', 'src/app/maen_client.cpp',
        'src/protection/protection.cpp', 'assets/companion/background.js',
        'assets/companion/background-1.9.7.js',
        '.github/workflows/maenbrowser-ci.yml', 'README.md', 'tools/preflight.ps1',
    ] + [p.relative_to(FILES).as_posix() for p in FILES.rglob('*') if p.is_file()]
    # Verify the most important internal consistency before exporting.
    if json.loads(manifest_path.read_text())['version'] != NEW_VERSION:
        raise RuntimeError('Manifest version mismatch')
    if f'project(MaenBrowser VERSION {NEW_VERSION}' not in cmake.read_text():
        raise RuntimeError('CMake version mismatch')
    if f'OutFile "MaenBrowser-{NEW_VERSION}-Setup.exe"' not in (root / 'installer/MaenBrowser.nsi').read_text():
        raise RuntimeError('Installer output name mismatch')
    return sorted(set(changed))


def zip_paths(zip_path: Path, folder: Path, items: list[str]):
    with zipfile.ZipFile(zip_path, 'w', compression=zipfile.ZIP_DEFLATED, compresslevel=8) as z:
        for rel in sorted(items):
            z.write(folder / rel, rel)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    source = parser.add_mutually_exclusive_group(required=True)
    source.add_argument('--repo', type=Path, help='Existing local MaenBrowser 1.9.6 checkout')
    source.add_argument('--clone', action='store_true', help='Clone the current public GitHub main branch with git')
    parser.add_argument('--output', type=Path, default=None, help='New export directory, defaults beside the checkout')
    args = parser.parse_args()
    with tempfile.TemporaryDirectory(prefix='maenbrowser-release-source-') as tmp:
        if args.clone:
            repo = Path(tmp) / 'checkout'
            subprocess.run(['git', 'clone', '--depth', '1', REPOSITORY, str(repo)], check=True)
        else:
            repo = args.repo.resolve()
        if not (repo / 'CMakeLists.txt').is_file():
            raise RuntimeError(f'Invalid repository root: {repo}')
        output = (args.output or repo.parent / 'MaenBrowser-1.9.7-EXPORT').resolve()
        if output.exists():
            raise RuntimeError(f'Output already exists; choose a clean new directory: {output}')
        if output == repo or repo in output.parents:
            raise RuntimeError('Export directory cannot be inside the source repository')
        staging = Path(tmp) / 'patched'
        shutil.copytree(repo, staging, ignore=shutil.ignore_patterns(
            '.git', '.vs', '.vscode', 'build', 'dist', 'node_modules', '*.pdb', '*.obj', '*.exe', '*.dll'))
        changed = patch(staging)
        output.mkdir(parents=True)
        try:
            upload = output / 'GitHub-Upload-Changed-Files'
            for rel in changed:
                dest = upload / rel
                dest.parent.mkdir(parents=True, exist_ok=True)
                shutil.copy2(staging / rel, dest)
            zip_paths(output / 'MaenBrowser-1.9.7-Changed-Files-Upload.zip', staging, changed)
            whole = [p.relative_to(staging).as_posix() for p in staging.rglob('*') if p.is_file()]
            zip_paths(output / 'MaenBrowser-1.9.7-Full-Source.zip', staging, whole)
            digests = {p: hashlib.sha256((staging / p).read_bytes()).hexdigest() for p in changed}
            (output / 'SHA256-MANIFEST.json').write_text(json.dumps(digests, indent=2) + '\n', encoding='utf-8')
            (output / 'UPLOAD_INSTRUCTIONS_AR.txt').write_text(
                'MaenBrowser 1.9.7\n'
                'ارفع الملفات داخل مجلد GitHub-Upload-Changed-Files بنفس مساراتها إلى المستودع، '\
                'ولا ترفع ملف ZIP وحده وتتوقع أن يحل محل الكود.\n'
                'بعد الرفع: شغّل اختبارات CI، ثم أنشئ tag باسم v1.9.7. '\
                'لن يُنشر Release إذا لم يتوفر CEF media runtime الموثّق.\n'
                'مهم: مستخدمو 1.9.6 يحتاجون تثبيتًا يدويًا مرة واحدة لتفعيل إشعارات التحديثات القادمة.\n',
                encoding='utf-8')
            print('READY:', output)
            print('FILES:', len(changed), 'changed/new upload paths')
            for rel in changed: print(' +', rel)
        except Exception:
            shutil.rmtree(output, ignore_errors=True)
            raise

if __name__ == '__main__':
    try: main()
    except (OSError, RuntimeError, subprocess.CalledProcessError) as exc:
        print('FAILED:', exc, file=sys.stderr)
        sys.exit(1)
