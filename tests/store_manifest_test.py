from pathlib import Path
from xml.etree import ElementTree as E
root=Path(__file__).resolve().parents[1];tree=E.parse(root/'installer/msix/AppxManifest.xml.in')
protocols=tree.findall('.//{http://schemas.microsoft.com/appx/manifest/uap/windows10/3}Protocol')
assert {p.attrib['Name'] for p in protocols}=={'http','https'}
assert all(p.attrib['Parameters']=='"%1"' for p in protocols)
client=(root/'src/app/maen_app.cpp').read_text(encoding='utf-8');assert 'protection::BlockNavigation(url)' in client and 'OpenUrlInNewTab(url)' in client
print('Store manifest: explicit quoted HTTP/HTTPS associations use existing guarded native URL routing; no default-app coercion')
