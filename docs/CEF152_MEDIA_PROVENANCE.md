# CEF 152 media build provenance

MaenBrowser 1.5.32 pins:

- CEF: `152.0.6+g708dc14+chromium-152.0.7977.83`
- CEF branch: `7977`
- CEF checkout: `708dc14`
- Target: Windows x64
- Media switches: `proprietary_codecs=true`, `ffmpeg_branding=Chrome`
- Release switches: `is_official_build=true`, `is_component_build=false`, `chrome_pgo_phase=0`

The same exact CEF/Chromium line and proprietary-codec approach is publicly used by
the Karere project for WhatsApp media. Its published CEF 152 binaries are Linux-only,
so they are intentionally NOT copied into this Windows project. This repository builds
the Windows x64 equivalent and verifies the exact CEF version before packaging it.

The final acceptance criterion is real playback of the target WhatsApp MP4 in the
resulting MaenBrowser runtime; a manifest or codec capability string alone is not
treated as proof of decoded playback.
