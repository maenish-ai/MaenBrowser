"""Offline package validation: assets, identities, immutable filter snapshots."""
import hashlib
import json
import pathlib
import re
import base64
import subprocess

root = pathlib.Path(__file__).resolve().parents[1]
companion = root / 'assets/companion'
manifest = json.loads((companion / 'manifest.json').read_text())
assert manifest['version'] == (root / 'VERSION').read_text().strip()
digest = hashlib.sha256(base64.b64decode(manifest['key'])).hexdigest()[:32]
extension_id = ''.join(chr(ord('a') + int(c, 16)) for c in digest)
assert extension_id in (root / 'src/protection/identity.h').read_text()
assert 'unsafe-eval' not in manifest['content_security_policy']['extension_pages']
assert not manifest.get('externally_connectable')
assert not manifest.get('web_accessible_resources')
for page in companion.glob('*.html'):
    html = page.read_text()
    assert not re.search(r'\son\w+\s*=', html, re.I), f'Inline event handler: {page.name}'
    assert not re.search(r'<script(?![^>]*\bsrc=)[^>]*>\s*\S', html, re.I), f'Inline script: {page.name}'
    for ref in re.findall(r'(?:src|href)="([^"#]+)"', html):
        if '://' not in ref and not ref.startswith('data:'):
            assert (companion / ref).is_file(), f'Missing asset {page.name}: {ref}'
for script in companion.glob('*.js'):
    subprocess.run(['node', '--check', str(script)], check=True)
for item in json.loads((root / 'assets/filters/manifest.json').read_text()):
    content = (root / 'assets/filters' / (item['name'] + '.txt')).read_bytes()
    assert hashlib.sha256(content).hexdigest() == item['sha256']
    domains = content.decode().splitlines()
    assert domains == sorted(set(domains))
    assert len(domains) == item['domains']
    assert all(re.fullmatch(r'[a-z0-9.-]+', d) and '.' in d for d in domains)
print(f'Package validation passed: version {manifest["version"]}, extension {extension_id}')
