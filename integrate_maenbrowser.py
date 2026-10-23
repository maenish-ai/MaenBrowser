#!/usr/bin/env python3
"""Apply the unified always-on MaenBrowser hardening to the reviewed 1.9.6 tree.

Python 3.10+ (stdlib only). No administrator rights required.
  python integrate_maenbrowser.py --repo PATH --check
  python integrate_maenbrowser.py --repo PATH --output-dir OUT
  python integrate_maenbrowser.py --clone --output-dir OUT

--clone requires git and network access; it clones the public GitHub project.
--output-dir writes full changed files, source ZIP, upload ZIP, and git patch.
A ZIP uploaded directly as ONE FILE to GitHub does NOT update source files.
"""
from __future__ import annotations
import argparse
import difflib
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import zipfile

SOURCE_REPO = "https://github.com/maenish-ai/MaenBrowser.git"
BUNDLE = Path(__file__).resolve().parent
VERSION = "2026-10-22-security-integration-2"

REPLACEMENTS: dict[str, list[tuple[str, str]]] = {
    "src/protection/protection.cpp": [
        (
            '#include "src/protection/filter_snapshot.h"\n',
            '#include "src/protection/filter_snapshot.h"\n'
            '#include "src/protection/always_on_shield.h"\n',
        ),
        (
            'struct AdRules { DomainRules domains; std::string error; };\n'
            'std::atomic<std::shared_ptr<const AdRules>> g_ad_rules{std::make_shared<const AdRules>()};\n'
            'std::atomic<uint64_t> g_blocked{0};',
            'struct AdRules { DomainRules domains; std::string error; };\n'
            'std::atomic<std::shared_ptr<const AdRules>> g_ad_rules{std::make_shared<const AdRules>()};\n'
            '// Native security policy: independent of ads, exceptions and family mode.\n'
            'struct SecurityRules { DomainRules domains; std::string error; };\n'
            'std::atomic<std::shared_ptr<const SecurityRules>> g_security_rules{std::make_shared<const SecurityRules>()};\n'
            'std::atomic<uint64_t> g_security_blocked{0};\n'
            'std::atomic<uint64_t> g_blocked{0};',
        ),
        (
            '    d->SetString("blockedRequests", std::to_string(g_blocked.load()));\n',
            '    d->SetString("blockedRequests", std::to_string(g_blocked.load()));\n'
            '    d->SetBool("securityShieldEnabled", true);\n'
            '    d->SetString("securityBlockedRequests", std::to_string(g_security_blocked.load()));\n'
            '    d->SetInt("securityThreatDomains", static_cast<int>(g_security_rules.load()->domains.Size()));\n'
            '    d->SetString("securityListError", g_security_rules.load()->error);\n',
        ),
        (
            'void Initialize() {\n'
            '  CefRegisterSchemeHandlerFactory("https", "maen.browser", new NoNetworkFactory());\n'
            '  auto ads = std::make_shared<AdRules>();',
            'void Initialize() {\n'
            '  CefRegisterSchemeHandlerFactory("https", "maen.browser", new NoNetworkFactory());\n'
            '  // Optional snapshot only adds reviewed indicators. Built-in Candiru and\n'
            '  // AdFection loader signatures remain active if this file is missing.\n'
            '  auto security = std::make_shared<SecurityRules>();\n'
            '  security->error = LoadDomains("security-threats.txt", security->domains);\n'
            '  g_security_rules.store(security);\n'
            '  auto ads = std::make_shared<AdRules>();',
        ),
        (
            '  CefRefPtr<CefResourceHandler> GetResourceHandler(CefRefPtr<CefBrowser>, CefRefPtr<CefFrame>, CefRefPtr<CefRequest> r) override {\n',
            '  void OnResourceRedirect(CefRefPtr<CefBrowser>, CefRefPtr<CefFrame>,\n'
            '      CefRefPtr<CefRequest>, CefRefPtr<CefResponse>, CefString& new_url) override {\n'
            '    // Re-evaluate the redirected URL BEFORE the destination is loaded.\n'
            '    // A previously benign URL may redirect to an attack endpoint.\n'
            '    if (internal_ || !Block(new_url.ToString(), source_, document_)) return;\n'
            '    denied_ = true;\n'
            '    // This hostname is registered to a local, non-network scheme handler.\n'
            '    new_url = "https://maen.browser/blocked";\n'
            '  }\n'
            '  CefRefPtr<CefResourceHandler> GetResourceHandler(CefRefPtr<CefBrowser>, CefRefPtr<CefFrame>, CefRefPtr<CefRequest> r) override {\n',
        ),
        (
            'bool BlockNavigation(const std::string& url) {\n',
            'bool BlockKnownSecurityThreat(const std::string& url) {\n'
            '  const auto u = ParseUrl(url);\n'
            '  if (Own(u) || !SecurityThreatUrl(u.host, u.path, g_security_rules.load()->domains))\n'
            '    return false;\n'
            '  ++g_security_blocked; ++g_blocked;\n'
            '  return true;\n'
            '}\n'
            'bool BlockNavigation(const std::string& url) {\n',
        ),
        (
            'bool Block(const std::string& url, const std::string& source, bool document) {\n'
            '  const auto s = Current();\n'
            '  if (!s->family && !s->ads) return false;\n'
            '  const auto u = ParseUrl(url);\n'
            '  if (Own(u)) return false;\n'
            '  bool denied = false;',
            'bool Block(const std::string& url, const std::string& source, bool document) {\n'
            '  const auto u = ParseUrl(url);\n'
            '  if (Own(u)) return false;\n'
            '  // Always-on attack prevention is evaluated first. No ads setting or\n'
            '  // ad exception can suppress it. No blocking of generic ad networks.\n'
            '  if (SecurityThreatUrl(u.host, u.path, g_security_rules.load()->domains)) {\n'
            '    ++g_security_blocked;\n'
            '    ++g_blocked;\n'
            '    return true;\n'
            '  }\n'
            '  const auto s = Current();\n'
            '  if (!s->family && !s->ads) return false;\n'
            '  bool denied = false;',
        ),
        (
            '  if (SecurityThreatUrl(u.host, u.path, g_security_rules.load()->domains)) {\n'
            '    ++g_security_blocked;\n'
            '    ++g_blocked;\n'
            '    return true;\n'
            '  }\n'
            '  const auto s = Current();',
            '  if (BlockKnownSecurityThreat(url)) return true;\n'
            '  const auto s = Current();',
        ),
    ],
    "src/protection/protection.h": [
        (
            'bool Block(const std::string& url, const std::string& source, bool document);\n',
            'bool Block(const std::string& url, const std::string& source, bool document);\n'
            '// Use the same always-on indicators for download admission, independent of ads.\n'
            'bool BlockKnownSecurityThreat(const std::string& url);\n',
        ),
    ],
    "src/app/maen_client.h": [
        (
            '  bool OnBeforeDownload(CefRefPtr<CefBrowser> browser,\n',
            '  bool CanDownload(CefRefPtr<CefBrowser> browser,\n'
            '                   const CefString& url, const CefString& request_method) override;\n'
            '  bool OnBeforeDownload(CefRefPtr<CefBrowser> browser,\n',
        ),
    ],
    "src/app/maen_client.cpp": [
        (
            '#include "src/protection/protection.h"\n',
            '#include "src/protection/protection.h"\n'
            '#include "src/protection/always_on_shield.h"\n'
            '#include "src/protection/download_guard.h"\n',
        ),
        (
            '  if (!browser || !browser->GetMainFrame()) return false;\n'
            '\n'
            '  // Route only explicit, same-origin HTTP(S) link clicks into the existing\n'
            '  // tab. Cross-origin popups may be required for sign-in or payment flows.\n'
            '  // Never navigate on script-generated popups without a user gesture.\n'
            '  const auto destination = protection::ParseUrl(target_url.ToString());\n'
            '  const auto source = protection::ParseUrl(browser->GetMainFrame()->GetURL().ToString());\n'
            '  if (!user_gesture || !source.valid || !destination.valid ||\n'
            '      (destination.scheme != "https" && destination.scheme != "http") ||\n'
            '      source.scheme != destination.scheme || source.host != destination.host ||\n'
            '      protection::BlockNavigation(target_url.ToString())) {\n'
            '    return false;\n'
            '  }\n'
            '  const std::string url = target_url.ToString();',
            '  // CEF: TRUE cancels a popup. No script-created background windows.\n'
            '  if (!browser || !browser->GetMainFrame() ||\n'
            '      protection::BlockUnsolicitedPopup(user_gesture)) return true;\n'
            '  const std::string url = target_url.ToString();\n'
            '  // User-initiated about:blank is required by some OAuth/payment flows.\n'
            '  // Navigation from the popup remains subject to the native request guard.\n'
            '  if (url.empty() || url == "about:blank") return false;\n'
            '  const auto destination = protection::ParseUrl(url);\n'
            '  if (!destination.valid ||\n'
            '      (destination.scheme != "https" && destination.scheme != "http") ||\n'
            '      protection::BlockNavigation(url) || protection::Block(url, "", true)) return true;\n'
            '\n'
            '  // Preserve explicitly clicked cross-origin sign-in/payment windows.\n'
            '  const auto source = protection::ParseUrl(browser->GetMainFrame()->GetURL().ToString());\n'
            '  if (!source.valid || source.scheme != destination.scheme || source.host != destination.host)\n'
            '    return false;',
        ),
        (
            '  if (!callback || !download_item) return true;\n',
            '  if (!callback || !download_item) return true;\n'
            '  // CEF Chrome Runtime can hang a download if Continue is omitted.\n'
            '  // Network policy already denies known malicious origins. For executable\n'
            '  // file names, show a warning and force Save As (which can be cancelled).\n'
            '  const bool risky_file = protection::RiskyDownloadName(suggested_name.ToWString());\n'
            '  if (risky_file) {\n'
            '    HWND owner = browser ? browser->GetHost()->GetWindowHandle() : nullptr;\n'
            '    MessageBoxW(owner,\n'
            '        ui::Text(L"Warning: This download can run programs or scripts. Review its source, and cancel the Save As dialog if you do not trust it.",\n'
            '                 L"تحذير: قد يشغّل هذا الملف برامج أو سكربتات. تحقق من المصدر وألغِ نافذة الحفظ إن لم تكن تثق به."),\n'
            '        L"MaenBrowser — Download security", MB_OK | MB_ICONWARNING);\n'
            '  }\n',
        ),
        (
            '  const bool ask = protection::Current()->ask_download || download_path.empty();\n',
            '  const bool ask = risky_file || protection::Current()->ask_download || download_path.empty();\n',
        ),
        (
            'bool MaenClient::OnBeforeDownload(CefRefPtr<CefBrowser> browser,\n',
            '// CEF calls CanDownload only for applicable user/renderer initiated flows.\n'
            '// Native request filtering is still required for all other flows.\n'
            'bool MaenClient::CanDownload(CefRefPtr<CefBrowser>, const CefString& url,\n'
            '                             const CefString&) {\n'
            '  return !protection::BlockKnownSecurityThreat(url.ToString());\n'
            '}\n'
            'bool MaenClient::OnBeforeDownload(CefRefPtr<CefBrowser> browser,\n',
        ),
    ],
    "CMakeLists.txt": [
        (
            'add_test(NAME domain_rules COMMAND MaenDomainTests)\n',
            'add_test(NAME domain_rules COMMAND MaenDomainTests)\n'
            'add_executable(MaenAdFectionTests tests/adfection_guard_test.cpp)\n'
            'target_include_directories(MaenAdFectionTests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")\n'
            'add_test(NAME adfection_guard COMMAND MaenAdFectionTests)\n'
            'add_executable(MaenShieldTests tests/always_on_shield_test.cpp)\n'
            'target_include_directories(MaenShieldTests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")\n'
            'add_test(NAME always_on_shield COMMAND MaenShieldTests)\n',
        ),
        (
            'add_test(NAME always_on_shield COMMAND MaenShieldTests)\n',
            'add_test(NAME always_on_shield COMMAND MaenShieldTests)\n'
            'add_executable(MaenDownloadGuardTests tests/download_guard_test.cpp)\n'
            'target_include_directories(MaenDownloadGuardTests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")\n'
            'add_test(NAME download_guard COMMAND MaenDownloadGuardTests)\n',
        ),
    ],
    "src/media/webview2_embedded.cpp": [
        (
            '    settings->put_AreDevToolsEnabled(FALSE);\n'
            '  }\n',
            '    settings->put_AreDevToolsEnabled(FALSE);\n'
            '    // WebView2 SmartScreen is on by default on supported installations.\n'
            '    // Require reputation checking explicitly unless Windows policy disables it.\n'
            '    ComPtr<ICoreWebView2Settings8> reputation;\n'
            '    if (SUCCEEDED(settings.As(&reputation)) && reputation)\n'
            '      reputation->put_IsReputationCheckingRequired(TRUE);\n'
            '  }\n',
        ),
    ],
    "assets/companion/background.js": [
        (
            "await setProtectionBadge(s.listError?'!':s.ads?'✓':'—');",
            "await setProtectionBadge(s.securityShieldEnabled? (s.securityListError?'!':'✓'):'?');",
        ),
    ],
    "assets/companion/popup.js": [
        (
            "  $('connectionDetail').textContent=t('Panel {panel} · Browser {browser}',{panel:chrome.runtime.getManifest().version,browser:state.version});",
            "  $('connectionDetail').textContent=t('Panel {panel} · Browser {browser}',{panel:chrome.runtime.getManifest().version,browser:state.version}) +\n"
            "    ' · Security: ' + (state.securityShieldEnabled ? (state.securityListError ? 'ON (built-in only)' : 'ON') : 'UNKNOWN');",
        ),
        (
            "  try{await chrome.action.setBadgeText({text:state.listError?'!':state.ads?'✓':'—'});}catch{/* A badge failure must not disable working protection controls. */}",
            "  try{await chrome.action.setBadgeText({text:state.securityShieldEnabled?(state.securityListError?'!':'✓'):'?'});}catch{/* A badge failure must not disable working protection controls. */}",
        ),
    ],
    "assets/companion/options.js": [
        (
            "  $('lists').textContent=t('Local lists: {ads} ad/tracker domains, {adult} adult domains, {violence} violence domains.',{ads:state.adDomains,adult:state.adultDomains,violence:state.violenceDomains});",
            "  $('lists').textContent=t('Local lists: {ads} ad/tracker domains, {adult} adult domains, {violence} violence domains.',{ads:state.adDomains,adult:state.adultDomains,violence:state.violenceDomains}) +\n"
            "    ' | Always-on security: ' + (state.securityShieldEnabled ? (state.securityListError ? 'ON (bundled indicators unavailable)' : 'ON') : 'UNKNOWN');",
        ),
        (
            "    try{await chrome.action.setBadgeText({text:state.ads?'✓':'—'});}catch{}",
            "    try{await chrome.action.setBadgeText({text:state.securityShieldEnabled?(state.securityListError?'!':'✓'):'?'});}catch{}",
        ),
    ],
    "tests/protection_sync_regression_test.mjs": [
        (
            """for(const source of [worker,api,popup])assert(source.includes("listError?'!'"));
assert(client.includes('if (!user_gesture || !source.valid || !destination.valid'));
assert(client.includes('source.scheme != destination.scheme || source.host != destination.host'));
assert(client.includes('opener->GetMainFrame()->LoadURL(next_url)'));
console.log('Protection state synchronization and same-origin popup guards present');""",
            """// Native shield is never inferred from the optional ad-block switch.
assert(worker.includes('securityShieldEnabled'));
assert(popup.includes('securityShieldEnabled'));
assert(api.includes('listError'));
assert(client.includes('BlockUnsolicitedPopup(user_gesture)'));
assert(client.includes('protection::Block(url, "", true)'));
assert(client.includes('source.scheme != destination.scheme || source.host != destination.host'));
assert(client.includes('opener->GetMainFrame()->LoadURL(next_url)'));
console.log('Always-on security badge, state sync and user-gesture popup guards present');""",
        ),
    ],
    "docs/SECURITY.md": [
        (
            '## Permissions, downloads, extensions\n',
            '## Always-on native security vs. optional ad blocking\n'
            'MaenBrowser checks the documented Candiru validator/C&C hosts and specific\n'
            'AdFection script-loader endpoints regardless of the user\'s `ads` choice.\n'
            'Ad allowlists/exceptions cannot bypass these indicators. Ordinary advertising\n'
            'routes remain governed exclusively by the optional ad-blocker. All shared\n'
            'native CEF and optional WebView2 requests call the same `protection::Block`.\n'
            'Automatic non-user-gesture CEF popups are cancelled, and network redirects\n'
            'are re-evaluated against the native policy. The shield is a lightweight\n'
            'in-process matcher; it does not launch an always-on antivirus service.\n'
            'These are historically observed indicators, not universal malware detection.\n'
            'See `docs/ADFECTION_MITIGATION_AR.md` for limitations and acceptance tests.\n'
            '\n'
            '## Permissions, downloads, extensions\n',
        ),
    ],
}

NEW_FILES = (
    "src/protection/adfection_guard.h",
    "src/protection/always_on_shield.h",
    "src/protection/download_guard.h",
    "assets/filters/security-threats.txt",
    "tests/adfection_guard_test.cpp",
    "tests/always_on_shield_test.cpp",
    "tests/download_guard_test.cpp",
    "tests/php_compromise_security_test.py",
    "tests/always_on_security_policy_test.py",
    "docs/ADFECTION_MITIGATION_AR.md",
    "docs/SECURITY_PERFORMANCE_RELEASE_GATE_AR.md",
    "docs/PHP_COMPROMISED_SITE_DEFENSE_AR.md",
)


def expected_changes(repo: Path) -> tuple[dict[str, bytes], dict[str, bytes]]:
    """Build full replacement contents, checking exact source anchors beforehand."""
    after: dict[str, bytes] = {}
    before: dict[str, bytes] = {}
    if not (repo / "CMakeLists.txt").is_file() or not (repo / "src/protection/domain_rules.h").is_file():
        raise RuntimeError("Not a MaenBrowser repository root (CMakeLists/domain_rules missing).")
    for rel, changes in REPLACEMENTS.items():
        path = repo / rel
        original = path.read_bytes()
        newlines = "\r\n" if b"\r\n" in original else "\n"
        text = original.decode("utf-8").replace("\r\n", "\n")
        for old, new in changes:
            count = text.count(old)
            if count != 1:
                raise RuntimeError(f"{rel}: anchor mismatch ({count} occurrences), refusing partial/unsafe merge")
            text = text.replace(old, new, 1)
        before[rel] = original
        after[rel] = text.replace("\n", newlines).encode("utf-8")
    for rel in NEW_FILES:
        path = repo / rel
        if path.exists():
            raise RuntimeError(f"Refusing to overwrite existing file: {rel}")
        after[rel] = (BUNDLE / "files" / rel).read_bytes()
    return before, after


def write_transaction(repo: Path, before: dict[str, bytes], after: dict[str, bytes]) -> None:
    written: list[str] = []
    try:
        for rel, content in after.items():
            path = repo / rel
            path.parent.mkdir(parents=True, exist_ok=True)
            handle, tmp = tempfile.mkstemp(prefix=".maen-integrate-", dir=path.parent)
            try:
                with os.fdopen(handle, "wb") as f:
                    f.write(content)
                    f.flush()
                    os.fsync(f.fileno())
                os.replace(tmp, path)
            finally:
                if os.path.exists(tmp):
                    os.unlink(tmp)
            written.append(rel)
    except Exception:
        for rel in reversed(written):
            if rel in before:
                (repo / rel).write_bytes(before[rel])
            else:
                (repo / rel).unlink(missing_ok=True)
        raise


def zip_directory(out_zip: Path, root: Path, *, source: bool) -> None:
    with zipfile.ZipFile(out_zip, 'w', compression=zipfile.ZIP_DEFLATED, compresslevel=7) as z:
        for path in sorted(root.rglob('*')):
            if not path.is_file():
                continue
            rel = path.relative_to(root)
            if source and ('.git' in rel.parts or 'build' in rel.parts or
                           '__pycache__' in rel.parts or rel.name.endswith(('.exe', '.obj'))):
                continue
            z.write(path, str(rel).replace('\\', '/'))


def write_export(repo: Path, before: dict[str, bytes], after: dict[str, bytes], out: Path) -> None:
    out.mkdir(parents=True, exist_ok=True)
    upload = out / 'GitHub-Upload-Changed-Files'
    if upload.exists():
        raise RuntimeError(f"Upload folder already exists: {upload}. Choose a new output folder.")
    upload.mkdir()
    for rel, data in after.items():
        path = upload / rel
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(data)
    zip_directory(out / 'MaenBrowser-Changed-Files-Upload.zip', upload, source=False)
    zip_directory(out / 'MaenBrowser-Full-Source-Merged.zip', repo, source=True)
    # A readable fallback for audited Git-based upload workflows.
    patches = []
    for rel, content in sorted(after.items()):
        a = before.get(rel, b'').decode('utf-8').splitlines(keepends=True)
        b = content.decode('utf-8').splitlines(keepends=True)
        patches.extend(difflib.unified_diff(a, b, fromfile=f'a/{rel}', tofile=f'b/{rel}'))
    (out / 'maenbrowser-security-integration.patch').write_text(''.join(patches), encoding='utf-8')
    manifest = {
        'bundle': VERSION,
        'source_repository': SOURCE_REPO,
        'note': 'Windows compile/CEF runtime acceptance NOT validated by portable tests.',
        'modified': list(before),
        'added': [p for p in after if p not in before],
        'sha256': {rel: hashlib.sha256(data).hexdigest() for rel, data in after.items()}
    }
    (out / 'SHA256-MANIFEST.json').write_text(json.dumps(manifest, indent=2, ensure_ascii=False), encoding='utf-8')
    print(f'Exported {len(after)} full source files and merged source ZIP to: {out}')


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    group = parser.add_mutually_exclusive_group(required=True)
    group.add_argument('--repo', type=Path, help='existing extracted or cloned MaenBrowser root')
    group.add_argument('--clone', action='store_true', help='clone public GitHub main (requires network)')
    parser.add_argument('--check', action='store_true', help='inspect all exact source anchors; write nothing')
    parser.add_argument('--output-dir', type=Path, help='also export full changed files, complete source and patch')
    args = parser.parse_args()
    try:
        if args.clone:
            if args.check:
                raise RuntimeError('--clone is not useful with --check; use --repo instead')
            if not args.output_dir:
                raise RuntimeError('--clone requires --output-dir to preserve the Git checkout')
            out = args.output_dir.resolve()
            repo = out / 'MaenBrowser-Patched-Git-Checkout'
            if repo.exists():
                raise RuntimeError(f'Output checkout already exists: {repo}')
            out.mkdir(parents=True, exist_ok=True)
            subprocess.run(['git', 'clone', '--depth', '1', '--branch', 'main', SOURCE_REPO, str(repo)], check=True)
        else:
            repo = args.repo.resolve()
        if args.output_dir:
            out = args.output_dir.resolve()
            if out == repo or out.is_relative_to(repo):
                raise RuntimeError('The export directory must be outside the repository to avoid self-inclusion')
        before, after = expected_changes(repo)
        print(f'PASS: {len(before)} reviewed replacements and {len(after)-len(before)} new files.')
        if args.check:
            return 0
        write_transaction(repo, before, after)
        print('Merged security and performance release policy with the existing 1.9.6 code.')
        if args.output_dir:
            write_export(repo, before, after, args.output_dir.resolve())
        print('REQUIRED BEFORE RELEASE: python tools/run-checks.py ; Windows CMake/CTest, CEF runtime and 2/4/8 GiB acceptance.')
        return 0
    except (OSError, UnicodeError, RuntimeError, subprocess.CalledProcessError, ValueError) as error:
        print('STOP:', error, file=sys.stderr)
        return 1

if __name__ == '__main__':
    sys.exit(main())
