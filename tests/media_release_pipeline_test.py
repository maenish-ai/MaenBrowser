"""Guard against publishing an untested or codec-less release."""
from pathlib import Path
r=Path(__file__).resolve().parents[1]
s=(r/'.github/workflows/maenbrowser-ci.yml').read_text()
assert 'tags: ["v*"]' in s
assert 'Release tag must match VERSION' in s
assert 'preview releases are forbidden' in s
assert 'needs: [configuration, build-setup]' in s
assert '--verify-tag --draft' in s
assert 'browser_smoke.mjs' in s
assert 'Get-FileHash $setup -Algorithm SHA256' in s
cmake=(r/'CMakeLists.txt').read_text()
assert 'target_compile_options(${test_target} PRIVATE /UNDEBUG)' in cmake
assert 'MaenResourceTests)' in cmake
print('Media release gates: version, codecs, tested build, draft and checksum checks passed')
