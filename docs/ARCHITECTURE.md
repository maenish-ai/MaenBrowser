# MaenBrowser architecture — 1.0.0 foundation

## Product goals
- Native Windows browser shell, Chromium/CEF engine.
- Local-first profile: no MaenBrowser cloud account required.
- Standard tabs/address/navigation supplied initially by CEF Chrome runtime; custom Maen UI follows without changing the engine contract.
- External web extensions supported through a guarded extension manager.
- Low-memory mode implemented at the tab lifecycle layer, never by disabling Chromium security boundaries.
- Clean Windows installer/uninstaller and signed update path.

## Engine
Modern Windows channel uses a current CEF Standard Distribution with Chrome runtime enabled. CEF follows Chromium releases and exposes a stable embedding API.

## Memory strategy
Phase 1 establishes process/profile boundaries. Phase 2 adds a Maen Tab Lifecycle Manager:
1. mark inactive tabs,
2. freeze timers where safe,
3. serialize restorable state,
4. close/discard dormant renderer instances after the configured threshold,
5. recreate the tab when focused.

This deliberately avoids flags such as disabling site isolation, because reducing security to save RAM is not acceptable.

## Legacy Windows
Windows 7/8.1 cannot safely run a permanently current Chromium engine. Legacy support therefore requires a separately labeled legacy channel and must never be presented as equally secure as the modern channel.
