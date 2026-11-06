"""Export full source and changed-file ZIPs; never claims to build Windows binaries."""
import argparse
import re
import os
import tempfile
import hashlib
from pathlib import Path
import subprocess
import zipfile

root = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--output', type=Path, required=True)
parser.add_argument('--base', default='HEAD', help='Commit against which changed ZIP and patch are generated')
parser.add_argument('--label', default='', help='Optional safe artifact suffix, e.g. audit-review')
args = parser.parse_args()
if args.label and not re.fullmatch(r'[A-Za-z0-9][A-Za-z0-9._-]{0,63}',args.label):
    parser.error('Label must be a safe filename component')
out = args.output.resolve()
if out == root or root in out.parents:
    parser.error('Output must be outside the source tree')
out.mkdir(parents=True, exist_ok=True)
version = (root / 'VERSION').read_text().strip()
def git(*args):
    return subprocess.check_output(['git', *args], cwd=root)
base = git('rev-parse', '--verify', args.base + '^{commit}').decode().strip()
artifact_version = version + ('-' + args.label if args.label else '')
files = sorted(set(git('ls-files', '--cached', '--others', '--exclude-standard', '-z').decode().split('\0')) - {''})
files = [p for p in files if (root / p).is_file()]
manifest = root / 'SOURCE_MANIFEST_SHA256.txt'
manifest.write_text(''.join(f'{hashlib.sha256((root/p).read_bytes()).hexdigest()}  {p}\n' for p in files if p not in {'SOURCE_MANIFEST_SHA256.txt', 'SHA256SUMS.txt'}), encoding='utf-8')
(root/'SHA256SUMS.txt').write_text(f'{hashlib.sha256(manifest.read_bytes()).hexdigest()}  SOURCE_MANIFEST_SHA256.txt\n', encoding='utf-8')
changed = sorted(set(git('diff', '--name-only', base, '-z').decode().split('\0')) | set(git('ls-files', '--others', '--exclude-standard', '-z').decode().split('\0')) | {'SOURCE_MANIFEST_SHA256.txt','SHA256SUMS.txt'})
changed = [p for p in changed if p and (root/p).is_file()]
files = sorted(set(files) | {'SOURCE_MANIFEST_SHA256.txt','SHA256SUMS.txt'})
artifacts=[]
for label, names in [('Full-Source',files), ('Changed-Files-Upload',changed)]:
    target=out/f'MaenBrowser-{artifact_version}-{label}.zip'
    with zipfile.ZipFile(target,'w',zipfile.ZIP_DEFLATED) as archive:
        for name in names: archive.write(root/name,name)
    with zipfile.ZipFile(target) as archive:
        assert archive.testzip() is None
        if label == 'Full-Source':
            assert '.github/workflows/maenbrowser-ci.yml' in archive.namelist()
    artifacts.append(target)
# A temporary index includes text AND binary fixtures without touching the
# user's real index, staging state, branch or commit history.
with tempfile.TemporaryDirectory(prefix='maen-export-index-') as temporary:
    environment={**os.environ,'GIT_INDEX_FILE':str(Path(temporary)/'index')}
    subprocess.run(['git','read-tree',base],cwd=root,env=environment,check=True)
    subprocess.run(['git','add','--all','--','.'],cwd=root,env=environment,check=True)
    patch=subprocess.check_output(['git','diff','--cached','--binary',base],cwd=root,env=environment)
patch_file=out/f'MaenBrowser-{artifact_version}.patch'
patch_file.write_bytes(patch);artifacts.append(patch_file)
(out/'SHA256SUMS.txt').write_text(''.join(f'{hashlib.sha256(p.read_bytes()).hexdigest()}  {p.name}\n' for p in artifacts))
for p in artifacts: print(f'{p} ({p.stat().st_size} bytes)')
