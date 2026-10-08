# 1.6.7 integration notes
WebView2 remains the active engine (including WhatsApp video support).
Modernized native tab strip, close-tab control, Segoe UI toolbar, Home navigation, external HTTP(S) launch.
Legacy CEF sources remain in the repository for reference; they are not linked into the WebView2 executable.
This is NOT feature parity with the legacy browser. Downloads, extensions and adaptive resource controls require dedicated ports.
Build and media playback must be tested on Windows via GitHub Actions and real sites.
