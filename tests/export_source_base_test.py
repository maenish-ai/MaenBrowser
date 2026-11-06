"""Ensure review exports include already committed changes from a pinned base."""
from pathlib import Path
import os,shutil,subprocess,sys,tempfile,zipfile,hashlib
script=Path(__file__).resolve().parents[1]/'tools/export-source.py'
with tempfile.TemporaryDirectory() as directory:
 root=Path(directory)/'repo';root.mkdir();out=Path(directory)/'artifacts'
 def git(*args):return subprocess.check_output(['git',*args],cwd=root,stderr=subprocess.STDOUT).decode().strip()
 git('init','-q');git('config','user.name','Export test');git('config','user.email','export-test@example.invalid')
 (root/'tools').mkdir();shutil.copyfile(script,root/'tools/export-source.py')
 (root/'.github/workflows').mkdir(parents=True);(root/'.github/workflows/maenbrowser-ci.yml').write_text('name: fixture\n')
 (root/'VERSION').write_text('1.12.2\n');(root/'tracked.txt').write_text('before\n')
 git('add','.');git('commit','-qm','baseline');base=git('rev-parse','HEAD')
 (root/'tracked.txt').write_text('committed review change\n');git('add','.');git('commit','-qm','review')
 (root/'fixture.bin').write_bytes(bytes(range(256)))
 subprocess.run([sys.executable,str(root/'tools/export-source.py'),'--base',base,'--label','audit-review','--output',str(out)],check=True,stdout=subprocess.DEVNULL)
 changed=next(out.glob('*Changed*.zip'));full=next(out.glob('*Full*.zip'));patch=next(out.glob('*.patch'))
 with zipfile.ZipFile(changed) as z:
  assert z.read('tracked.txt')==b'committed review change\n'
  assert z.read('fixture.bin')==bytes(range(256))
 with zipfile.ZipFile(full) as z:
  for line in z.read('SOURCE_MANIFEST_SHA256.txt').decode().splitlines():
   digest,name=line.split('  ',1);assert hashlib.sha256(z.read(name)).hexdigest()==digest
 git('reset','--hard',base);git('clean','-fd')
 subprocess.run(['git','apply','--check',str(patch)],cwd=root,check=True)
 subprocess.run(['git','apply',str(patch)],cwd=root,check=True)
 assert (root/'tracked.txt').read_text()=='committed review change\n'
 assert (root/'fixture.bin').read_bytes()==bytes(range(256))
print('Export: pinned base includes committed changes and binary files; complete manifest and applicable patch verified')
