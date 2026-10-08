# 1.6.5 review

Source-level fixes: include windowsx.h for GET_X_LPARAM; null-check WebView2 navigation and refresh; guard Back/Forward; close controllers before releasing WebView2; handle popup deferral and failed controller creation; validate LOCALAPPDATA.

Important: The GitHub Actions run ID for the latest 1.6.4 push was not accessible via the connected GitHub API. These are static findings, not a verified diagnosis of that run. Windows compilation and media playback have not been performed in this environment.
