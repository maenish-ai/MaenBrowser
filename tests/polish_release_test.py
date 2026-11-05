from pathlib import Path
import json
r=Path(__file__).resolve().parents[1]
nsi=(r/'installer/MaenBrowser.nsi').read_bytes();assert nsi.startswith(b'\xef\xbb\xbf')
assert 'لغة تثبيت' in nsi.decode('utf-8-sig')
workflow=(r/'.github/workflows/maenbrowser-ci.yml').read_text(encoding='utf-8')
assert 'makensis /INPUTCHARSET UTF8 MaenBrowser.nsi' in workflow
client=(r/'src/app/maen_client.cpp').read_text(encoding='utf-8')
assert 'ExecuteChromeCommand(command,CEF_WOD_NEW_FOREGROUND_TAB)' in client
assert 'if(it->root!=root)continue;' in client
assert 'self->OpenUrlInNewTab(next_url,opener)' in client
assert 'client->OpenUrlInNewTab(url,opener)' in client
assert 'protection::BlockNavigation(next)' in client
manifest=json.loads((r/'assets/companion/manifest.json').read_text(encoding='utf-8'))
assert 'https://api.met.no' in manifest['content_security_policy']['extension_pages']
for path in (r/'.github/workflows').glob('*.yml'):
 s=path.read_text(encoding='utf-8');assert 'actions/setup-java@v4' not in s;assert 'actions/upload-artifact@v4' not in s
print('Polish: UTF8 installer source/build, same-window tab handoff, family checks, weather CSP and updated actions passed')
