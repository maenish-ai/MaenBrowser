"""Portable source/logic checks. Does not claim a Windows build or runtime test."""
import pathlib, subprocess, sys, shutil, tempfile
root=pathlib.Path(__file__).resolve().parents[1]
commands=[[sys.executable,'tools/verify-package.py']]
commands += [['node',str(p.relative_to(root))] for p in sorted((root/'tests').glob('*_test.mjs'))]
commands += [[sys.executable,str(p.relative_to(root))] for p in sorted((root/'tests').glob('*_test.py'))]
failed=[]
for command in commands:
 print('RUN', ' '.join(command),flush=True)
 if subprocess.run(command,cwd=root).returncode:failed.append(command[-1])
compiler=(shutil.which('g++') or shutil.which('clang++')) if sys.platform!='win32' else None
if compiler:
 with tempfile.TemporaryDirectory() as directory:
  for source in sorted((root/'tests').glob('*_test.cpp')):
   binary=str(pathlib.Path(directory)/source.stem)
   result=subprocess.run([compiler,'-std=c++20','-I.',str(source),'-o',binary],cwd=root)
   if result.returncode or subprocess.run([binary],cwd=root).returncode:failed.append(source.name)
else: print('Native portable tests: run by CTest in Windows CI (no local C++ compiler).')
print('FAILED: '+', '.join(failed) if failed else 'All available portable checks passed.',flush=True)
sys.exit(bool(failed))
