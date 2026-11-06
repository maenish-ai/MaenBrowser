"""Exercise the actual portable CMake guard: a library directory is not trust."""
import subprocess,tempfile
from pathlib import Path
root=Path(__file__).resolve().parents[1]
with tempfile.TemporaryDirectory() as directory:
 p=Path(directory)/'probe.cmake'
 for enabled in ['OFF','ON']:
  p.write_text(f'set(MAEN_ENABLE_UPDATER {enabled})\ninclude("{(root/"cmake/UpdatePolicy.cmake").as_posix()}")\n',encoding='utf-8')
  result=subprocess.run(['cmake','-P',str(p)],capture_output=True,text=True)
  if enabled=='OFF':assert result.returncode==0,result.stderr
  else:assert result.returncode!=0 and 'anti-rollback security floor' in result.stderr
print('Updater: OFF build remains available; unsafe optional ON configuration is rejected')
