# MaenBrowser1.12.2 — preserve download request ownership

The user reported downloads starting and immediately cancelling after recent
tab changes. Two app-controlled paths could disrupt a transfer: cancelling a
popup and replaying its destination as a fresh GET; and hiding/closing a popup
from download-progress callbacks. Signed redirects, POST bodies, referrers,
opener state and simultaneous downloads require the original context to live.
This identifies code risks, not a trace-based diagnosis of a specific private
GitHub download; Windows acceptance remains required.

Changes:
- OnBeforePopup keeps family checks, then lets Chromium keep the original
  request/WebContents. No cancel-and-replay or method/body/referrer replacement.
- The extension only reparents ordinary pages after top-level document commit.
  Attachment downloads do not commit documents, so their transfer windows stay
  untouched. Existing OAuth/payment checks, bounds and expiry remain.
- OnDownloadUpdated records progress only; it never hides/closes a tab/window.
  This also avoids hiding the main window after a former popup is reparented,
  or closing a window when another transfer in it is still active. An empty
  temporary download window may remain until Chromium/the user closes it.
- Valid CEF callbacks are continued once; invalid/missing callbacks delegate to
  the runtime rather than claiming a download without continuing it.
- Remove the dormant embedded-WebView2 fallback's unconditional download cancel.
  If that fallback is used, its own runtime handles downloads in its original
  profile using default download UI. It is not rerouted into CEF. This legacy
  surface remains off by default; its items are not promised in CEF's downloads
  list. TLS/family/request restrictions and runtime security stay in place.
- The downloads page shows Chromium's error code for an interrupted item instead
  of only a generic state. User Cancel, Pause and supported Resume remain.
- Add behavioral event tests and source-lifetime guards. Expand real Windows CEF
  smoke tests to POST target=_blank attachments and redirected attachments whose
  server requires the original referrer/body, with file-content verification.

Portable checks and PowerShell preflight passed locally; the new Windows download
cases must run after upload. No claim is made that they already passed in this
Linux workspace. Do not disable Safe Browsing, the sandbox, family rules or TLS,
copy cookies, auto-retry one-time POSTs, or auto-open downloaded files as a fix.
Version1.12.2 / Android source versionCode11202; no new Android behavior here.
