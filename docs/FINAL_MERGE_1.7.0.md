# MaenBrowser 1.7.0 — Embedded WebView2 + Audio Devices + Lite-on-Demand

This release keeps the original full MaenBrowser CEF 152 Chrome Runtime and fixes the three requested integration points without replacing the browser shell.

## 1. WhatsApp in the same MaenBrowser tab
- WhatsApp URLs are intercepted at main-frame navigation.
- WebView2 is created lazily as a child surface inside the current CEF tab content host.
- The previous separate MaenMediaHost top-level application is no longer built or launched.
- Left-click navigation therefore stays in the current MaenBrowser tab.
- Chrome Runtime's existing context menu / Open link in new tab flow remains available; the new tab receives its own lazy WebView2 surface when it navigates to WhatsApp.
- Navigating that tab back to a normal site immediately destroys the WebView2 surface and returns the tab to normal CEF browsing.

## 2. Speakers and microphones
- Audio output is provided by Microsoft Edge WebView2 and follows the Windows audio stack/default output device. This covers laptop speakers, 3.5 mm output, USB audio, Bluetooth audio and HDMI/DisplayPort when Windows exposes them.
- Microphone requests from WhatsApp Web are explicitly allowed in the embedded WebView2 permission handler. Windows privacy/device permissions still remain authoritative.
- Other sites are not automatically granted microphone access by this special WhatsApp rule.

## 3. Lightweight / older PCs
- WebView2 is not started at browser launch. It is created only when a routed media site is actually opened.
- Its controller and child window are destroyed as soon as that tab leaves the media site or closes.
- Existing Lite Mode detection remains active for <=6 GiB RAM or <=4 logical processors.
- Lite Mode disables prerender/back-forward cache/optimization hints/media router and uses small disk/media cache ceilings.
- GPU/video acceleration is not forcibly disabled, because doing so often increases CPU load on old PCs.

## Validation required
The source/package has been statically checked, but Windows compilation and real-device behavior must be verified by GitHub Actions and on a Windows machine. Acceptance checks: same-tab WhatsApp, right-click/new-tab behavior, video/audio playback, speaker switching through Windows, microphone voice note/call access, normal CEF sites, downloads, and memory use on an older PC.
