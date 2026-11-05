"""Check every Android SDK setup call, including reusable and signing jobs."""
from pathlib import Path
import re
root=Path(__file__).resolve().parents[1]
count=0
for path in (root/'.github/workflows').glob('*.yml'):
    source=path.read_text(encoding='utf-8')
    for match in re.finditer(r'(?m)^      - uses: android-actions/setup-android@v3\s*$',source):
        following=source[match.end():]
        step=re.split(r'(?m)^      - ',following,maxsplit=1)[0]
        packages=re.search(r"(?m)^          packages: ['\"]([^'\"]+)['\"]$",step)
        assert packages, f'{path.name}: explicit SDK packages required; defaults request retired tools'
        tokens=packages.group(1).split()
        assert 'tools' not in tokens, f'{path.name}: retired tools package'
        assert {'platform-tools','platforms;android-36','build-tools;35.0.0'}<=set(tokens)
        count+=1
assert count==3, f'Expected all three SDK setups, found {count}'
print('Android CI: all 3 SDK setup steps use supported explicit packages; no retired tools default')
