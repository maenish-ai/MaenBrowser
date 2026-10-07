# MaenBrowser 1.5.29 status

- Full Windows Setup-producing MaenBrowser source project.
- CEF 152 / Chromium 152 pinned.
- Verified custom CEF Media path remains the release path for H.264/AAC HTML5/MSE playback.
- Added native Windows Media Foundation H.264/AAC capability plumbing using OS MFTs; no codec pack or background service.
- Security sandbox, Windows process hardening, GPU/WebGL/WebRTC/MSE paths and adaptive Lite/Balanced/Performance policy remain intact.
- Setup-only distribution; no Portable artifact.

Runtime truth: the new Media Foundation layer does not magically replace CEF's HTML5 decoder. WhatsApp/Messenger H.264/AAC playback must be validated with the media-enabled CEF runtime; do not claim success before runtime testing.
