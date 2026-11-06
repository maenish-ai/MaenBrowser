"""Fetch a SHA-256 pinned codec-enabled CEF distribution; no stock fallback.

This verifies archive identity and layout, not successful video decoding. The
Windows browser integration test remains mandatory for distributing Setup.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path, PurePosixPath
import re
import shutil
import struct
import sys
import tarfile
import tempfile
from urllib.parse import urlparse
from urllib.request import urlopen
import zipfile

ROOT = Path(__file__).resolve().parents[1]
LOCK = ROOT / 'config/media-runtime.lock.json'
RECEIPT = 'MAEN_MEDIA_PROVENANCE.json'
REQUIRED = ('Release/libcef.dll', 'Release/libcef.lib', 'Release/bootstrap.exe',
            'include/cef_version.h', 'cmake/FindCEF.cmake', 'LICENSE.txt',
            'Resources/icudtl.dat', 'Resources/resources.pak', 'CREDITS.html',
            'Release/chrome_elf.dll', 'Release/d3dcompiler_47.dll',
            'Release/v8_context_snapshot.bin', 'Release/vk_swiftshader.dll',
            'Release/vk_swiftshader_icd.json', 'Release/vulkan-1.dll',
            'Release/dxil.dll', 'Release/dxcompiler.dll',
            'Resources/chrome_100_percent.pak', 'Resources/chrome_200_percent.pak',
            'Resources/locales/en-US.pak', 'Resources/locales/ar.pak')


def digest(path):
    h = hashlib.sha256()
    with Path(path).open('rb') as source:
        for block in iter(lambda: source.read(1024 * 1024), b''):
            h.update(block)
    return h.hexdigest()


def https_url(value):
    u = urlparse(value)
    if u.scheme != 'https' or not u.hostname or u.username or u.password:
        raise ValueError('CEF archive URL must be HTTPS without embedded credentials')
    return value


def load_lock(path=LOCK):
    lock = json.loads(Path(path).read_text(encoding='utf-8'))
    if lock.get('schema') != 1 or lock.get('platform') != 'windows64':
        raise ValueError('Unsupported runtime lock schema/platform')
    if not re.fullmatch(r'[0-9a-f]{64}', lock.get('sha256', '')):
        raise ValueError('Runtime lock requires SHA-256')
    if not re.fullmatch(r'\d+\.\d+\.\d+\+g[0-9a-f]+\+chromium-\d+\.\d+\.\d+\.\d+', lock.get('cef_version', '')):
        raise ValueError('Invalid CEF version')
    https_url(lock['url'])
    return lock


def selection(lock, url=None, sha=None):
    if bool(url) != bool(sha):
        raise ValueError('Set BOTH MAEN_CEF_ARCHIVE_URL and MAEN_CEF_ARCHIVE_SHA256, or neither')
    if not url:
        return {**lock, 'kind': 'locked-upstream'}
    if not re.fullmatch(r'[0-9a-fA-F]{64}', sha):
        raise ValueError('Custom CEF archive SHA-256 must contain 64 hex characters')
    return {**lock, 'url': https_url(url), 'sha256': sha.lower(), 'kind': 'custom-build'}


def check_archive(path, spec):
    if digest(path) != spec['sha256']:
        raise ValueError('CEF archive SHA-256 mismatch; refusing runtime')
    if spec['kind'] == 'locked-upstream' and path.stat().st_size != spec['size_bytes']:
        raise ValueError('CEF archive size mismatch')


def fetch(spec, cache):
    cache.mkdir(parents=True, exist_ok=True)
    suffix = '.zip' if urlparse(spec['url']).path.lower().endswith('.zip') else '.tar.bz2'
    target = cache / (spec['sha256'] + suffix)
    if target.exists():
        check_archive(target, spec)
        return target
    part = target.with_suffix(target.suffix + '.part')
    try:
        print('Downloading pinned codec-enabled CEF runtime...', file=sys.stderr)
        with urlopen(spec['url'], timeout=120) as response, part.open('wb') as output:
            https_url(response.geturl())
            shutil.copyfileobj(response, output, length=1024 * 1024)
        check_archive(part, spec)
        part.replace(target)
    finally:
        part.unlink(missing_ok=True)
    return target


def safe_member(name):
    path = PurePosixPath(name)
    if '\\' in name or ':' in name or path.is_absolute() or '..' in path.parts:
        raise ValueError('Unsafe CEF archive member: ' + name)


def unpack(path, destination):
    if zipfile.is_zipfile(path):
        with zipfile.ZipFile(path) as archive:
            for item in archive.infolist():
                safe_member(item.filename)
                if (item.external_attr >> 16) & 0o170000 == 0o120000:
                    raise ValueError('CEF archive symlinks are forbidden')
            archive.extractall(destination)
    else:
        with tarfile.open(path, 'r:bz2') as archive:
            for item in archive.getmembers():
                safe_member(item.name)
                if not (item.isfile() or item.isdir()):
                    raise ValueError('CEF archive special files are forbidden')
            archive.extractall(destination, filter='data')


def layout(root, version):
    for relative in REQUIRED:
        if not (root / relative).is_file():
            raise ValueError('CEF runtime missing ' + relative)
    header = (root / 'include/cef_version.h').read_text(encoding='utf-8')
    if f'#define CEF_VERSION "{version}"' not in header:
        raise ValueError('CEF runtime version does not match application lock')
    # Ensure a Windows x64 runtime, not ARM64 or an accidental Linux archive.
    with (root / 'Release/libcef.dll').open('rb') as source:
        if source.read(2) != b'MZ':
            raise ValueError('libcef.dll is not a Windows PE binary')
        source.seek(0x3c)
        offset_bytes = source.read(4)
        if len(offset_bytes) != 4:
            raise ValueError('libcef.dll PE header is truncated')
        offset = struct.unpack('<I', offset_bytes)[0]
        source.seek(offset)
        if source.read(6) != b'PE\0\0\x64\x86':
            raise ValueError('libcef.dll is not Windows x64')


def prepare(archive, destination, spec):
    check_archive(archive, spec)
    destination.mkdir(parents=True, exist_ok=True)
    # Never reuse a directory by version alone: stock/media builds share versions.
    target = destination / ('media-' + spec['sha256'])
    with tempfile.TemporaryDirectory(prefix='maen-cef-', dir=destination) as temporary:
        staging = Path(temporary)
        unpack(archive, staging)
        roots = [p.parent.parent for p in staging.glob('*/cmake/FindCEF.cmake')]
        if len(roots) != 1:
            raise ValueError('Expected exactly one CEF distribution root')
        root = roots[0]
        layout(root, spec['cef_version'])
        if spec['kind'] == 'custom-build':
            marker = (root / 'MAEN_MEDIA_RUNTIME.txt').read_text(encoding='utf-8')
            for expected in [f"cef_version={spec['cef_version']}", *spec['codec_build_flags']]:
                if expected not in marker:
                    raise ValueError('Custom media build marker missing: ' + expected)
        receipt = {key: spec[key] for key in ('kind', 'url', 'sha256', 'cef_version')}
        receipt['url'] = urlparse(spec['url'])._replace(query='', fragment='').geturl()
        receipt['files'] = {p: digest(root / p) for p in REQUIRED}
        (root / RECEIPT).write_text(json.dumps(receipt, indent=2) + '\n', encoding='utf-8')
        if target.exists():
            shutil.rmtree(target)
        shutil.move(str(root), target)
    return target.resolve()


def verify(root, spec):
    layout(root, spec['cef_version'])
    receipt = json.loads((root / RECEIPT).read_text(encoding='utf-8'))
    for key in ('kind', 'url', 'sha256', 'cef_version'):
        expected = urlparse(spec[key])._replace(query='', fragment='').geturl() if key == 'url' else spec[key]
        if receipt.get(key) != expected:
            raise ValueError('CEF provenance mismatch: ' + key)
    for relative in REQUIRED:
        if receipt.get('files', {}).get(relative) != digest(root / relative):
            raise ValueError('CEF extracted file changed: ' + relative)


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--destination', type=Path, default=ROOT/'third_party/cef')
    p.add_argument('--lock',type=Path,default=LOCK,help='Explicit isolated candidate lock; never changes the production lock')
    p.add_argument('--archive', type=Path, help='Use a local archive, verifying the selected SHA-256')
    p.add_argument('--verify', type=Path, help='Verify a previously prepared runtime')
    args = p.parse_args()
    spec = selection(load_lock(args.lock), os.environ.get('MAEN_CEF_ARCHIVE_URL'), os.environ.get('MAEN_CEF_ARCHIVE_SHA256'))
    if args.verify:
        verify(args.verify, spec)
        print('CEF archive provenance/layout verified; real playback still requires Windows tests.', file=sys.stderr)
    else:
        archive = args.archive or fetch(spec, args.destination/'downloads')
        print(prepare(archive, args.destination, spec))


if __name__ == '__main__':
    try:
        main()
    except (ValueError, OSError, KeyError, tarfile.TarError, zipfile.BadZipFile) as error:
        sys.exit('Media runtime error: ' + str(error))
