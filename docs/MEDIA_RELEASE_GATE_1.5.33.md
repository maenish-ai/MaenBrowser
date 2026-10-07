# MaenBrowser 1.5.33 media release gate

This release intentionally has **no stock CEF fallback**.

A Setup artifact is produced only when repository variables
`MAEN_CEF_ARCHIVE_URL` and `MAEN_CEF_ARCHIVE_SHA256` point to the exact verified
Windows x64 CEF 152 media archive produced by `tools/build-cef-media.ps1`.

If those variables are absent, CI stops and produces no Setup. This prevents a
green stock-CEF build from being mistaken for a WhatsApp-media build.

The CEF media build is pinned to branch 7977 / checkout 708dc14 /
CEF 152.0.6+g708dc14+chromium-152.0.7977.83 and requires
`proprietary_codecs=true` and `ffmpeg_branding=Chrome`.

A successful build/marker is not itself a claim that a particular WhatsApp file
plays. Actual WhatsApp playback is the final acceptance test.
