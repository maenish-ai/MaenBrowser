"""Export full source and changed-file ZIPs; never claims to build Windows binaries."""
import argparse
import os
import tempfile
import hashlib
from pathlib import Path
import subprocess
import zipfile

root = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--output', type=Path, required=True)
args = parser.parse_args()
out = args.output.resolve()
if out == root or root in out.parents:
    parser.error('Output must be outside the source tree')
out.mkdir(parents=True, exist_ok=True)
version = (root / 'VERSION').read_text().strip()
def git(*args):
    return subprocess.check_output(['git', *args], cwd=root)
files = sorted(set(git('ls-files', '--cached', '--others', '--exclude-standard', '-z').decode().split('\0')) - {''})
files = [p for p in files if (root / p).is_file()]
manifest = root / 'SOURCE_MANIFEST_SHA256.txt'
manifest.write_text(''.join(f'{hashlib.sha256((root/p).read_bytes()).hexdigest()}  {p}\n' for p in files if p not in {'SOURCE_MANIFEST_SHA256.txt', 'SHA256SUMS.txt'}), encoding='utf-8')
(root/'SHA256SUMS.txt').write_text(f'{hashlib.sha256(manifest.read_bytes()).hexdigest()}  SOURCE_MANIFEST_SHA256.txt\n', encoding='utf-8')
changed = sorted(set(git('diff', '--name-only', 'HEAD', '-z').decode().split('\0')) | set(git('ls-files', '--others', '--exclude-standard', '-z').decode().split('\0')) | {'SOURCE_MANIFEST_SHA256.txt','SHA256SUMS.txt'})
changed = [p for p in changed if p and (root/p).is_file()]
artifacts=[]
for label, names in [('Full-Source',files), ('Changed-Files-Upload',changed)]:
    target=out/f'MaenBrowser-{version}-{label}.zip'
    with zipfile.ZipFile(target,'w',zipfile.ZIP_DEFLATED) as archive:
        for name in names: archive.write(root/name,name)
    with zipfile.ZipFile(target) as archive:
        assert archive.testzip() is None
        assert '.github/workflows/maenbrowser-ci.yml' in archive.namelist()
    artifacts.append(target)
# A temporary index includes text AND binary fixtures without touching the
# user's real index, staging state, branch or commit history.
with tempfile.TemporaryDirectory(prefix='maen-export-index-') as temporary:
    environment={**os.environ,'GIT_INDEX_FILE':str(Path(temporary)/'index')}
    subprocess.run(['git','read-tree','HEAD'],cwd=root,env=environment,check=True)
    subprocess.run(['git','add','--all','--','.'],cwd=root,env=environment,check=True)
    patch=subprocess.check_output(['git','diff','--cached','--binary','HEAD'],cwd=root,env=environment)
patch_file=out/f'MaenBrowser-{version}.patch'
patch_file.write_bytes(patch);artifacts.append(patch_file)
(out/'SHA256SUMS.txt').write_text(''.join(f'{hashlib.sha256(p.read_bytes()).hexdigest()}  {p.name}\n' for p in artifacts))
for p in artifacts: print(f'{p} ({p.stat().st_size} bytes)')
