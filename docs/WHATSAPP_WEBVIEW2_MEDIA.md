# WhatsApp Web media in MaenBrowser 1.7.0

WhatsApp Web remains inside the MaenBrowser window. The normal browser is the full CEF 152 Chrome Runtime; when a tab navigates to WhatsApp Web, MaenBrowser lazily creates a WebView2 child surface inside that tab's native content host. This provides the Microsoft Edge media stack without opening a second top-level application.

The WebView2 environment is shared and created only on first use. A tab-specific controller is destroyed when that tab leaves WhatsApp or closes. Audio output follows devices exposed by Windows. WhatsApp microphone requests are allowed by the WebView2 permission handler, while Windows privacy controls remain authoritative.

The installer bundles the Microsoft WebView2 Evergreen bootstrapper. GitHub Actions still builds and publishes Setup only.
