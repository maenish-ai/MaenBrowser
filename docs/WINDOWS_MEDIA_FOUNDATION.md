# Windows Media Foundation integration

MaenBrowser 1.5.29 adds a native Windows Media Foundation capability layer that
probes the OS-provided H.264 and AAC Media Foundation Transforms. It does not
install third-party codec packs and it does not weaken the CEF sandbox.

Important architectural boundary: CEF owns Blink HTML5 video, MSE, blob URLs,
WebRTC and the Chromium media pipeline. An application-side MFT probe cannot
replace Chromium's decoder for a WhatsApp `<video>` element. H.264/AAC
recognition inside that pipeline still requires a CEF/Chromium build with
`proprietary_codecs=true`; the project's verified Media CEF path additionally
uses `ffmpeg_branding=Chrome` for the Chrome codec configuration.

This layer is therefore retained as native capability plumbing/diagnostics and
as a future platform fallback foundation, not as a false claim that stock CEF
can play proprietary WhatsApp media.
