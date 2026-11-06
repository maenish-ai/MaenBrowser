"""Security boundaries for the opt-in agent path and expanded local filtering."""
from pathlib import Path
import json
r=Path(__file__).resolve().parents[1]
app=(r/'src/app/maen_app.cpp').read_text(encoding='utf-8')
client=(r/'src/app/maen_client.cpp').read_text(encoding='utf-8')
native=(r/'src/protection/protection.cpp').read_text(encoding='utf-8')
assert 'storage::IsAgentMode() && (protection::FamilyEnabled()' in app
assert app.index('storage::IsAgentMode() && (protection::FamilyEnabled()') < app.index('CefBrowserHost::CreateBrowser')
assert 'AppendSwitchWithValue("remote-debugging-address", "127.0.0.1")' in app
assert 'RemoveSwitch("remote-allow-origins")' in app
assert 'op != "get" && op != "technical"' in native
assert 'storage::SettingsChanged(agent_settings_stamp_, now)' in client
assert 'CloseBrowser(true)' in client
assert app.index('client_->PrepareAgentProtection()') < app.index('protection::Initialize()')
assert '!client_->AgentPolicyUnchanged()' in app
assert native.count('MergeBundledAdRules(')>=3
assert 'MergeBundledAdRules(*ads);g_ad_rules.store(ads)' in native
assert 'SelectAdShim(state->family,state->ads,true' in native
assert 'SetString("version", kVersion)' in native
ads=set((r/'assets/filters/ads.txt').read_text(encoding='utf-8').splitlines())
extra=set((r/'assets/filters/ads-extra.txt').read_text(encoding='utf-8').splitlines())
assert len(ads|extra)>len(ads)
for host in ['www.facebook.com','web.whatsapp.com','accounts.google.com','photoshop.adobe.com']:
    assert not any(host==h or host.endswith('.'+h) for h in ads|extra),host
formats=json.loads((r/'tests/fixtures/formats.json').read_text(encoding='utf-8'))
assert any(f['file']=='h264-high.mp4' and f.get('required',True) for f in formats)
assert any(f['file']=='sample.wma' and not f.get('required',True) for f in formats)
print('Compatibility release: local agent isolation/family guards, merged filters, critical sites and honest codec gates passed')

assert 'OnAlreadyRunningAppRelaunch' in app
assert 'OpenUrlInNewTab(url)' in app
main=(r/'src/main_win.cpp').read_text(encoding='utf-8')
assert 'CEF_RESULT_CODE_NORMAL_EXIT_PROCESS_NOTIFIED) return 0;' in main
