# MaenBrowser 1.0 development feature matrix

MaenBrowser uses CEF Chrome Runtime so that mature Chromium browser UI and web-platform behavior are reused instead of reimplementing insecure imitations.

## Integrated in this source tree
- Chromium/CEF Chrome Runtime window and standard browser chrome.
- Multi-tab/window support supplied by Chrome Runtime.
- Back/forward/reload/address/search behavior supplied by Chrome Runtime.
- Downloads with Save As interception plus completion actions (Open / Show in folder).
- Persistent local profile, cookies, cache and user preferences.
- Private/incognito capability through Chrome Runtime UI/profile behavior.
- Standard Chromium dialogs/permissions where exposed by CEF Chrome Runtime.
- PDF viewing/printing capabilities provided by the CEF/Chromium runtime where supported.
- Unpacked-extension loader for CEF-supported extension APIs.
- Automatic Lite Mode on <= 6 GB RAM: disables prerender and back-forward cache; keeps sandbox and site isolation.
- No MaenBrowser cloud account or proprietary sync service.
- Windows per-user installer, shortcuts, Installed Apps registration and full-data uninstall.
- GitHub Actions build producing installer and portable artifacts.

## Not falsely claimed as complete
- Chrome Web Store one-click compatibility is not guaranteed.
- Google Sync is intentionally disabled.
- A custom password-manager UI is not implemented by MaenBrowser.
- Signed automatic update feed is not active until release signing/feed infrastructure exists.
- RAM superiority versus every Chrome/Edge/Firefox workload is not guaranteed; it must be benchmarked.
- Android is a separate future implementation.

## Performance rule
Memory reductions must not disable the Chromium sandbox, site isolation, TLS validation, or other core security boundaries. Lite Mode favors removing speculative caching/pre-render work before considering more invasive process changes.
