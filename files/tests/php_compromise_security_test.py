"""Check server-compromise mitigations without pretending to repair PHP servers.

Runtime and integration behavior still require Windows+CEF and WebView2 tests.
"""
from pathlib import Path
r = Path(__file__).resolve().parents[1]
p = (r / 'src/protection/protection.cpp').read_text(encoding='utf-8')
ph = (r / 'src/protection/protection.h').read_text(encoding='utf-8')
client = (r / 'src/app/maen_client.cpp').read_text(encoding='utf-8')
wv = (r / 'src/media/webview2_embedded.cpp').read_text(encoding='utf-8')
header = (r / 'src/protection/download_guard.h').read_text(encoding='utf-8')
assert 'BlockKnownSecurityThreat(const std::string& url)' in p
assert 'BlockKnownSecurityThreat(const std::string& url)' in ph
assert 'if (BlockKnownSecurityThreat(url)) return true;' in p
assert 'if (!s->family && !s->ads) return false;' in p
assert p.index('if (BlockKnownSecurityThreat(url)) return true;') < p.index('if (!s->family && !s->ads) return false;')
assert 'MaenClient::CanDownload' in client and 'BlockKnownSecurityThreat(url.ToString())' in client
assert 'RiskyDownloadName(suggested_name.ToWString())' in client
assert 'MB_OK | MB_ICONWARNING' in client
assert 'risky_file || protection::Current()->ask_download || download_path.empty()' in client
assert 'callback->Continue(download_path, ask);' in client
assert 'ICoreWebView2Settings8' in wv
assert 'put_IsReputationCheckingRequired(TRUE)' in wv
assert 'RiskyDownloadName(std::wstring_view' in header
resource = (r / 'src/app/resource_mode.cpp').read_text(encoding='utf-8')
assert 'disable-background-mode' in resource and 'disk-cache-size' in resource
assert not any(needle in resource for needle in ('no-sandbox', 'disable-web-security', 'ignore-certificate-errors'))
print('Compromised-site protections: download warnings + forced Save As, URL gate, WebView2 SmartScreen, legacy performance policy present')
