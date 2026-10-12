# MaenBrowser 1.9.6 — stability, resource policy and media release gates

Windows x64 C++20 / CEF Chrome Runtime browser. This is the complete **source
project**, not a prebuilt Setup. Portable checks passed during preparation;
**Windows compilation, real CEF integration, device performance and external-site
acceptance remain pending**. See [validation](docs/VALIDATION_1.9.6.md).

## Start here
- [Arabic upload/build guide](START_HERE_AR.md)
- [Changes and precise scope](docs/RELEASE_1.9.6.md)
- [Windows acceptance checklist](docs/ACCEPTANCE_1.9.6.md)
- [Custom CEF media runtime](docs/CUSTOM_CEF_MEDIA_BUILD.md)

## Changes
- Clean search homepage; ad blocking remains controlled by the bundled extension.
- Native revision checks reject stale or revisionless writes; settings panels send
  only changed fields. Badge/popup state refreshes on settings events.
- UltraLite profile for 2 GB-class hardware; manual resource override after restart.
  Bounded idle-tab scans react to memory pressure, keeping existing safety exclusions.
- Explicit saved-session restore loads one site, leaving other tabs as local placeholders.
- Family domain lists load only when family protection is enabled.
- Manual, bounded HTTPS ad-filter refresh from the existing StevenBlack source;
  validated atomic replacement with last-good/bundled fallback.
- DPAPI-protected settings backup; explicit PIN recovery keeps family protection on.
- Sandbox-required startup/build, explicit confirmation for a small external-protocol
  allowlist, shared window icons and reuse of the native request handler.
- Synthetic codec fixtures extend the Windows playback test to VP8/Vorbis,
  VP9/Opus, AV1/Opus, MP3, FLAC, WAV, Ogg Vorbis and AAC alongside H.264.

## Build Windows Setup
Extract the archive and upload its **contents** to the repository root, including
`.github/workflows`. Uploading the ZIP alone does not update the application source.
The workflow **Build MaenBrowser Windows Setup** creates an installable stock-CEF
preview when the two optional media-runtime secrets are absent. That preview does
not enable or verify H.264 playback. For a verified H.264 Setup, configure both
repository secrets:

- `MAEN_CEF_ARCHIVE_URL`: HTTPS URL of the verified, version-matched media CEF ZIP.
- `MAEN_CEF_ARCHIVE_SHA256`: exact SHA-256 of that archive.

The stock-CEF preview and H.264 Setup are uploaded as differently named artifacts.
If only one secret is set or either value is malformed, configuration fails instead of
silently selecting a runtime. The separate media runtime workflow needs an online
Windows x64 self-hosted build runner. Source changes cannot supply the publisher's
archive, credentials, codec licensing review or signing certificate.

The Setup workflow performs source checks, sandboxed C++ build and native tests before
creating `MaenBrowser-1.9.6-Setup.exe`. The in-tab playback test runs only with the
verified custom CEF archive. A green stock-CEF preview workflow does not establish H.264
support, device performance or all third-party service compatibility.
No GitHub changes or workflow runs were made as part of preparing this source archive.

## Local source checks
```
python tools/run-checks.py
```
On Windows, with VS 2022, CMake, Node 22, Python, NSIS, the CEF secrets in environment
variables and `WEBVIEW2_SDK_ROOT` set, run `tools/package-nsis.ps1` from the repo root.
Normal browsing/media uses CEF. The optional manual WebView2 window requires an
already-installed WebView2 Runtime; Setup no longer silently installs it.

## Limits
Targets supported Windows x64 environments, not legacy Windows 7/8 or 32-bit Windows.
Codec support is tested, not inferred from `canPlayType` alone. HEVC/DRM and service
acceptance are separate platform/licensing concerns. No universal codec, extension,
service or 2 GB performance guarantee is made. Filter updates are manual, not a
background service. Security engine updates remain an ongoing release responsibility.
