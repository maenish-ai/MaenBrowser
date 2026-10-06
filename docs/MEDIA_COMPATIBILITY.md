# MaenBrowser media compatibility policy

MaenBrowser treats modern media playback as a release-blocking compatibility surface.

## Required web media surface

Release testing covers MP4/H.264/AVC + AAC, WebM VP8/VP9 + Opus/Vorbis, AV1, MP3, FLAC, WAV, Media Source Extensions (MSE), WebRTC, WebCodecs and common browser-supported image formats. HEVC/H.265 is opportunistic/platform-dependent and must not be advertised as universal. DRM playback is a separate CDM/licensing surface.

## Performance and security rules

- Keep Chromium GPU acceleration and accelerated video decoding enabled by default.
- Never use `--ignore-gpu-blocklist` as a production compatibility shortcut.
- Never disable sandbox, site isolation or certificate validation for media compatibility.
- Prefer hardware decoding when Chromium/Windows exposes a safe supported path; retain Chromium fallbacks.
- Do not ship random codec packs or replace Chromium FFmpeg binaries at runtime.
- Do not preload media services in the background.

## CEF distribution boundary

The public CEF binary downloaded by `tools/fetch-cef.ps1` is the reproducible CI baseline. A production distribution that enables additional proprietary codecs must use a separately built and verified CEF/Chromium runtime with the appropriate Chromium GN media configuration and must pass licensing review before redistribution.

The custom CEF build must remain ABI/version matched to the pinned CEF branch. Do not mix `libcef.dll`, resources, locales or FFmpeg artifacts from another Chromium/CEF version.

## Release gate

A release is not declared media-ready solely because `canPlayType()` reports support. Manual/runtime tests must include:

1. WhatsApp Web received MP4 video: thumbnail, open, play/pause, seek and audio.
2. WhatsApp Web download of the same media.
3. H.264/AAC MP4 test asset.
4. VP9/Opus WebM and AV1 test asset.
5. YouTube playback controls, resolution menu and subtitles.
6. WebRTC audio/video call smoke test.
7. 4 GiB Windows machine: 1080p playback without MaenBrowser-specific background overhead; 4K is capability-dependent and is never guaranteed on unsupported hardware.

If H.264/AAC playback is absent, the release remains blocked rather than silently falling back to an untrusted codec package.
