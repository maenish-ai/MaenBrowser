# MaenBrowser 1.1 development feature matrix

## Integrated
- Chromium/CEF 152 web engine with Chrome Runtime compatibility.
- MaenBrowser-branded Windows executable, installer and shortcuts.
- MaenBrowser start page with lightweight search and quick links.
- Multi-tab/window navigation supplied by the CEF Chrome Runtime.
- Persistent local profile, cookies, cache and preferences.
- Downloads delegated to Chromium UI plus MaenBrowser completion actions.
- Automatic Lite Mode on systems with <= 6 GB RAM without disabling sandbox/site isolation.
- Per-user NSIS installer and full-data uninstall.
- GitHub Actions Windows x64 build.

## Deliberate boundaries
- The current browser frame still uses CEF Chrome Runtime. A fully custom tab/address-bar shell is a later architectural step, not falsely claimed here.
- Programmatic unpacked-extension loading is disabled because the legacy request-context extension APIs are absent in pinned CEF 152.
- Chrome Web Store one-click compatibility is not guaranteed.
- Google Sync is disabled; there is no proprietary MaenBrowser cloud account.
- A MaenBrowser password-manager UI and signed update feed are not yet implemented.
- RAM superiority is not claimed without benchmarks.
- Android remains a separate future implementation.

## Design rule
MaenBrowser may adopt useful browser interaction patterns, but it does not copy third-party browser branding, icons or proprietary visual assets.
