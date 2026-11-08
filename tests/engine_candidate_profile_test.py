"""Offline provenance checks; no claim that the candidate was built or is secure."""
from pathlib import Path
import json,re
root=Path(__file__).resolve().parents[1]
p=json.loads((root/'config/cef-upgrade-candidate.json').read_text(encoding='utf-8'))
lock=json.loads((root/'config/media-runtime.lock.json').read_text(encoding='utf-8'))
m=re.fullmatch(r'(\d+)\.\d+\.\d+\+g([0-9a-f]{7,40})\+chromium-\d+\.\d+\.(\d+)\.\d+',p['cef_version'])
assert m and p['checkout'].startswith(m.group(2)) and p['branch']==m.group(3)
assert re.fullmatch('[0-9a-f]{40}',p['checkout'])
assert re.fullmatch('[0-9a-f]{64}',p['automate_sha256'])
assert p['automate_url']==f"https://raw.githubusercontent.com/chromiumembedded/cef/{p['checkout']}/tools/automate/automate-git.py"
assert p['status']=='unbuilt-candidate-not-production' and p['cef_version']!=lock['cef_version']
workflow=(root/'.github/workflows/build-cef-upgrade.yml').read_text(encoding='utf-8')
assert p['checkout'][:7] in workflow and 'test-cef-candidate.ps1' in workflow
assert 'self-hosted' in workflow and 'workflow_dispatch' in workflow
print('CEF candidate provenance and isolation: pinned source, branch, builder directory, runtime smoke and unchanged active lock')
