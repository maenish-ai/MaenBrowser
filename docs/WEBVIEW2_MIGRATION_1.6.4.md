# WebView2 native browser experimental build 1.6.5

This is a Windows-only architectural migration candidate. CEF is no longer linked into the executable. WebView2 Runtime supplies web rendering and media decoding. No codec DLLs are redistributed.

New-window requests are attached to new WebView2 tab controls in the same top-level window using NewWindowRequested deferrals.

Known limitations: no full bookmarks/downloads/history/extension management UI; limited tab management (tab switching and creation, no close UI); no automated Windows compilation performed here; runtime testing and security review required before publication. Existing CEF profile is not automatically migrated.
