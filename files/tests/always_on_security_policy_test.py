"""Portable invariant checks; Windows CEF and live exploit tests are still required."""
from pathlib import Path
r = Path(__file__).resolve().parents[1]
source = (r / 'src/protection/protection.cpp').read_text(encoding='utf-8')
popup = (r / 'src/app/maen_client.cpp').read_text(encoding='utf-8')
manifest = (r / 'assets/filters/security-threats.txt').read_text(encoding='utf-8').splitlines()
worker = (r / 'assets/companion/background.js').read_text(encoding='utf-8')
assert 'securityShieldEnabled' in source
assert 'securityBlockedRequests' in source
assert 'securityListError' in source
assert 'OnResourceRedirect(' in source
block = source[source.index('bool Block(const std::string& url'):source.index('std::string Rewrite(')]
assert block.index('BlockKnownSecurityThreat(url)') < block.index('if (!s->family && !s->ads) return false;')
assert 's->exceptions' not in block[:block.index('BlockKnownSecurityThreat(url)')]
assert 'BlockUnsolicitedPopup(user_gesture)' in popup
assert 'protection::Block(url, "", true)' in popup
assert 'securityShieldEnabled' in worker
assert "s.ads?'✓'" not in worker
assert len(manifest) == 6 and manifest == sorted(manifest)
assert 'fearevolve.com' in manifest and 'keyadapter.com' in manifest
assert not any(x in manifest for x in ('mtrtb.com', 'akedo.gg', 'volasim.com', 'google.com'))
assert 'disk-cache-size' in (r / 'src/app/resource_mode.cpp').read_text(encoding='utf-8')
assert 'USE_SANDBOX' in (r / 'CMakeLists.txt').read_text(encoding='utf-8')
print('Always-on security, independent ads state, minimal resource and IOC scope tests passed')
