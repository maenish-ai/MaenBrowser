# MaenBrowser 1.6.8 — Media Release Candidate

This release retains the WebView2 playback path verified by the user on WhatsApp Web.
No new codec DLLs are bundled: the Microsoft Edge WebView2 Runtime supplies the browser media stack.

## Windows acceptance checklist

- Install the GitHub Actions Setup on a clean Windows 10/11 x64 device.
- Confirm MaenBrowser launches and its local start page appears.
- Open WhatsApp Web and play both video and audio, including seeking and fullscreen.
- Test HTML5 MP4 H.264/AAC and WebM VP9/Opus media on independent non-YouTube websites.
- Test AV1 where supported by the Windows hardware/runtime.
- Test popup video players: the new page should remain inside a tab.
- Test downloads, back/forward, new tab, close tab, relaunch and clean uninstall.
- Record sites that fail and classify DRM, unsupported codecs, access restrictions or website bugs.

This is a release candidate for testing, not a guarantee that every media codec or DRM service works.
Legacy CEF files are preserved for reference, not compiled into the executable.
