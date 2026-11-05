from pathlib import Path
r=Path(__file__).resolve().parents[1]
app=(r/'src/app/maen_app.cpp').read_text(encoding='utf-8')
assert 'GetLocalizedString' in app and 'IDS_APP_MENU_YOUR_CHROME_HEADER' in app
assert 'L"About MaenBrowser"' in app and 'IDS_PRODUCT_NAME' in app
client=(r/'src/app/maen_client.cpp').read_text(encoding='utf-8')
command=client[client.index('bool MaenClient::OnChromeCommand'):client.index('CefRefPtr<CefResourceRequestHandler>')]
assert 'IDC_SHOW_HISTORY' in command and 'OpenUrlInNewTab(' in command
assert 'GetMainFrame()->LoadURL' not in command
assert 'internal && IsTrustedInternalTab(url)' in client
for path in (r/'.github/workflows').glob('*.yml'):
 s=path.read_text(encoding='utf-8');assert 'upload-artifact@v5' not in s and 'download-artifact@v5' not in s
print('Refinement: native resource branding, menu new-tab routing, narrow internal permissions and Node24 artifact actions passed')
