# MaenBrowser 1.6.9 — Full Browser + WebView2 Media

This package intentionally restores/preserves the full MaenBrowser CEF 152 desktop browser
(UI/runtime behavior, start page, downloads, resource mode, preferences, security hardening,
extensions scaffold, profile storage, updater scaffold and NSIS integration) and adds the
Microsoft Edge WebView2 media path for WhatsApp Web.

Architecture:
- MaenBrowser.exe: original/full CEF 152 browser.
- MaenMediaHost.exe: WebView2-based WhatsApp media/browser surface.
- src/media/webview2_media_router.*: routes WhatsApp Web main-frame navigation from CEF to
  MaenMediaHost.
- MicrosoftEdgeWebview2Setup.exe: downloaded by GitHub Actions and bundled by Setup to ensure
  the Evergreen WebView2 Runtime is present.

The main browser has NOT been replaced by the simplified WebView2-only prototype from 1.6.4+.
That prototype proved WebView2 media compatibility on the test PC; this branch uses WebView2
only where the CEF codec boundary matters.

Release acceptance test on Windows:
1. Build the Setup through GitHub Actions.
2. Install/upgrade MaenBrowser.
3. Confirm original MaenBrowser UI/settings/start page remain.
4. Open WhatsApp Web from MaenBrowser and sign in if necessary.
5. Play multiple received/sent videos with audio.
6. Confirm ordinary sites still open through the normal CEF browser.
7. Confirm downloads and existing browser functions still work.

No claim is made that every codec/DRM format on every website is supported. WebView2 uses the
installed Microsoft Edge WebView2 Runtime and therefore substantially broadens mainstream media
compatibility compared with stock CEF, subject to the runtime, website and DRM policies.
