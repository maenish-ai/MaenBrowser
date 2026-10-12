# Current media policy — 1.9.6

Normal playback remains in CEF in the current tab. Release builds require the
verified version-matched media CEF archive described in CUSTOM_CEF_MEDIA_BUILD.md.
Stock-codec fallback cannot produce the release Setup.

Required automated samples: H.264/AAC MP4; VP8/Vorbis, VP9/Opus and AV1/Opus WebM;
MP3, FLAC, PCM WAV, Ogg Vorbis and AAC/M4A. tests/browser_smoke.mjs verifies playback
progress and video dimensions. A canPlayType claim alone is insufficient.

The automated samples do not prove audible device output, commercial DRM services,
HEVC support, platform-specific WebCodecs or the operation of every third-party
website. These need the manual acceptance matrix in ACCEPTANCE_1.9.6.md.
No unsafe codec packs, GPU blocklist overrides, TLS exceptions or automatic second
engine routing were added. Manual WebView2 compatibility requires an already
installed Runtime and a separate session; it is optional, not the codec guarantee.

Documentation: https://www.chromium.org/audio-video/
