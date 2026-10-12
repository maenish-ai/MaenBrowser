"""Structural release invariants; not Windows execution evidence."""
from pathlib import Path
import re
r=Path(__file__).resolve().parents[1]
main=(r/'src/main_win.cpp').read_text(encoding="utf-8")
assert 'settings.no_sandbox = true' not in main
assert 'if (!sandbox_info)' in main and 'return 1;' in main
assert 'startup->HasSwitch(flag)' in main
native=(r/'src/protection/protection.cpp').read_text(encoding="utf-8")
save=native[native.index('if (op != "save")'):]
assert save.index('expectedRevision') < save.index('Import(patch')
assert 'HasKey("expectedRevision") &&' not in save
assert 'recovered->family = true' in native
assert 'VerifyPin(*recovered' in native
assert 'ParseAdSnapshot(text, rules->domains)' in native
assert native.index('ParseAdSnapshot(text, rules->domains)') < native.index('SaveAdSnapshot(text)')
workflow=(r/'.github/workflows/maenbrowser-ci.yml').read_text(encoding="utf-8")
assert 'browser_smoke.mjs' in workflow and ' -RequireMedia' in workflow
assert 'Setup-stock-CEF-preview' in workflow
assert 'Setup-H264' in workflow
assert 'if: needs.configuration.outputs.media_ready == \'true\'' in workflow
assert 'if: needs.configuration.outputs.media_ready != \'true\'' in workflow
assert 'does not include the verified proprietary-codec runtime' in workflow
assert 'MicrosoftEdgeWebview2Setup.exe' not in workflow
installer=(r/'installer/MaenBrowser.nsi').read_text(encoding="utf-8")
assert 'RMDir /r "$LOCALAPPDATA\\MaenBrowser"' not in installer
print('Release source invariants passed; Windows runtime validation still required')
