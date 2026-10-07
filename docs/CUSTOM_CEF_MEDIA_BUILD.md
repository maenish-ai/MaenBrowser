# MaenBrowser custom CEF media build

MaenBrowser's normal CI uses the official CEF binary distribution. Official/default CEF does not enable Chrome proprietary codecs. WhatsApp Web MP4/H.264 playback therefore cannot be made equivalent to Chrome merely by adding application command-line switches.

For a version-matched custom CEF build, use the CEF automated build process for the Chromium 152 / CEF 152 release branch and build with:

```
GN_DEFINES=is_official_build=true proprietary_codecs=true ffmpeg_branding=Chrome chrome_pgo_phase=0
```

`tools/build-cef-media.ps1` records these required build arguments. Building Chromium/CEF requires substantial disk, RAM and build time and is intentionally not run on the normal 60-minute GitHub Windows application job.

## Integration

Package the resulting CEF binary distribution with the same directory layout as a CEF binary distribution. Configure the application build with two environment variables:

- `MAEN_CEF_ARCHIVE_URL`: HTTPS URL of the reviewed custom CEF archive.
- `MAEN_CEF_ARCHIVE_SHA256`: exact 64-hex SHA-256 of that archive.

`tools/fetch-cef.ps1` refuses an unverified custom archive. The CEF version must remain compatible with MaenBrowser's pinned `152.0.6+g708dc14+chromium-152.0.7977.83` line.

## Release acceptance

Before public distribution, test the exact runtime, not only `canPlayType()`:

1. WhatsApp Web MP4/H.264 video displays a frame/thumbnail.
2. Play/pause and seeking work and audio/video remain synchronized.
3. Download works independently of playback.
4. YouTube playback and controls work.
5. VP9/AV1/WebM, MSE, WebRTC and WebCodecs remain functional.
6. Hardware acceleration remains enabled where Chromium considers the GPU safe.
7. Test low-memory/older hardware separately.

## Licensing boundary

Enabling `ffmpeg_branding=Chrome proprietary_codecs=true` changes the codec set and may create patent/licensing obligations for redistribution. Open-source source code does not itself remove those obligations. Obtain appropriate licensing/legal clearance before distributing such binaries publicly.

## Maen Media Engine integration (1.5.24)

MaenBrowser deliberately reuses the Chromium/CEF media architecture rather than embedding a second Gecko/Firefox engine. The custom runtime is built from the CEF/Chromium source line already used by MaenBrowser, with the Chrome FFmpeg branding and proprietary-codec build gates enabled. The browser keeps Chromium GPU selection, MSE, WebRTC, WebCodecs and software fallback behavior intact; no GPU blocklist bypass or security weakening is introduced.

The media-runtime workflow also emits a build manifest and SHA-256. This proves the archive came from the intended build configuration; it is not a claim that a particular WhatsApp file has decoded successfully. Runtime acceptance still requires the real playback tests listed above.
