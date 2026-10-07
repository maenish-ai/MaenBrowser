# WhatsApp media path (1.6.0)

MaenBrowser 1.6.0 uses a hybrid Windows media architecture:

- The normal browser remains native C++ with pinned CEF 152 and the existing CEF sandbox/security/resource policy.
- Navigation to `web.whatsapp.com` is routed to `MaenMediaHost.exe`, a native Win32 WebView2 host branded as MaenBrowser.
- The host uses Microsoft's Evergreen WebView2 Runtime (Edge/Chromium web platform), avoiding the proprietary-codec limitation of stock CEF.
- Setup carries Microsoft's small Evergreen bootstrapper and runs it silently. Windows 11 and most eligible Windows 10 systems already have the runtime; Microsoft services it independently.
- WhatsApp WebView2 profile data is local at `%LOCALAPPDATA%\MaenBrowser\WebView2 Media` and is removed by full MaenBrowser uninstall because the existing uninstall removes `%LOCALAPPDATA%\MaenBrowser`.
- No custom codec DLL, codec pack, external player, TLS bypass, GPU disable, or sandbox weakening is used.

This architecture is specifically intended to restore modern WhatsApp HTML5 media compatibility without requiring the project to build a proprietary-codec CEF/Chromium distribution. Runtime acceptance still requires testing the user's actual WhatsApp video after installing the generated Setup.
