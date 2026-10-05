# MaenBrowser 1.5.8 feature matrix

## Integrated and buildable
- CEF/Chromium 152 web engine with production sandbox enabled in CI.
- Familiar Chromium tab/navigation behavior through Chrome Runtime.
- MaenBrowser branding, title branding and Windows icon assets.
- Lightweight local start page with user-selectable Google, Bing, DuckDuckGo or Brave Search.
- Quick Access links for YouTube, Gmail, WhatsApp and Wikipedia without background preloading.
- Persistent local profile, cookies, cache and preferences.
- Downloads through Chromium UI plus MaenBrowser completion actions.
- History, PDF, printing, picture-in-picture and private-window capabilities where exposed by the pinned Chromium runtime.
- Three-tier resource policy:
  - Lite: <= 6 GB RAM; disables prerender + BFCache and caps caches.
  - Balanced: 6–16 GB; disables prerender and uses moderate cache caps.
  - Performance: >= 16 GB; stays close to Chromium defaults.
- Windows Setup installer, Installed Apps entry, Start/Desktop shortcuts, App Paths and browser/default-app capability registration.
- Full uninstall option removes MaenBrowser local profile data.
- No mandatory Maen account or proprietary cloud service.

## Product principles accepted for the custom-shell phase
These are requirements, but are not falsely marked as implemented until the custom MaenBrowser shell replaces the Chrome-style shell:
- Maen-owned tab strip and omnibox.
- Sleeping/discarding inactive tabs with audio/form/download safeguards.
- Tab groups/stacks, optional vertical tabs and workspaces.
- Split view.
- Saved sessions.
- Maen-owned Bookmarks/History/Downloads surfaces.
- Reader mode.
- Quick command palette spanning tabs/history/bookmarks/actions.
- Resource dashboard and per-site/tab memory controls.
- Extension management on a CEF-152-supported API path.

## Security/performance gate
A feature is rejected or made optional if its steady-state RAM/CPU cost is disproportionate.
Memory savings must never disable the Chromium sandbox, site isolation, TLS validation or other core security boundaries.

## Compatibility boundary
The current desktop engine is the pinned modern CEF/Chromium Windows x64 line. It does not claim safe support for every obsolete Windows release.
Android requires a separate native mobile implementation and is not included in this Windows source package.
