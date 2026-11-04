"""Check integration ordering around the independently tested origin policy."""
from pathlib import Path
r=Path(__file__).resolve().parents[1]
app=(r/'src/app/maen_app.cpp').read_text()
assert app.index('RegisterLocalEndpoint(global)') < app.index('CefBrowserHost::CreateBrowser(')
assert app.index('RegisterLocalEndpoint(context)') < app.index('CefBrowserHost::CreateBrowser(')
s=(r/'src/protection/protection.cpp').read_text()
assert 'frame ? frame->GetURL().ToString()' in s
assert 'if (trusted_) r->SetHeaderByName("Origin", kExtensionOrigin, true)' in s
assert '"Access-Control-Allow-Origin", kExtensionOrigin' in s
assert '"Access-Control-Allow-Origin", "null"' not in s
assert '"Access-Control-Allow-Origin", "*"' not in s
assert 'RegisterSchemeHandlerFactory(\n      "https", "maen.browser"' in s
print('Internal endpoint integration: startup registration order, actual-frame checks and exact CORS origin passed')
