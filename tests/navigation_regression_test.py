"""Source guard for the unsafe native console-to-navigation bridge.
Windows integration checks are separately required for actual CEF callbacks.
"""
from pathlib import Path
r=Path(__file__).resolve().parents[1]
s=(r/'src/app/maen_client.cpp').read_text()
assert 'MAEN_MEDIA_UNSUPPORTED' not in s
assert 'media_fallbacks_' not in s
assert 'InstallMediaProbe' not in s
assert 'IDC_ABOUT' in s and 'ControlsUrl("about.html")' in s
start=s.index('void MaenClient::OnLoadEnd')
end=s.index('void MaenClient::OnTitleChange',start)
load=s[start:end]
assert load.index('media_navigations_.erase(pending)') < load.index('media::OpenEmbeddedWebView2')
print('Native navigation source guards passed (not a Windows runtime test)')
