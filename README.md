# MaenBrowser 1.9.10 — codec-enabled CEF in normal Windows builds

Normal builds automatically fetch a pinned **CEF 154 Windows x64 H.264/AAC**
distribution. No runtime secrets or self-hosted Chromium build are needed.
Stock-CEF fallback is removed. A successful Setup build must pass actual in-tab
codec playback tests, including H.264/AAC MP4, pause and seek.

- [Arabic upload/build/release instructions](UPLOAD_AND_RELEASE_AR.md)
- [Internal API fix and validation limits](docs/RELEASE_1.9.10.md)
- [Research and dependency provenance](docs/MEDIA_RUNTIME_SOURCES.md)
- [Exact runtime URL and SHA-256](config/media-runtime.lock.json)

Upload the extracted source contents, including `.github`, to the repository
root. Push main or run **Build MaenBrowser Windows Setup**. If old
`MAEN_CEF_ARCHIVE_URL` / `MAEN_CEF_ARCHIVE_SHA256` secrets select CEF 152, remove
both to select the committed codec runtime. Overrides remain optional and must
match the new exact version and media marker.

The workflow downloads headers/libraries/runtime together, verifies SHA-256,
version, archive layout and Windows x64 architecture, builds with sandboxing,
runs native tests and real codec playback, then packages Setup and a checksum.
Pushing `v1.9.10` creates a draft GitHub release only after those gates pass.
Application profiles are preserved by the existing installer upgrade behavior.

Local checks: `python tools/run-checks.py` (Node and a C++ compiler recommended).
Windows build: VS2022, CMake, Node22, Python3.12+, NSIS and the pinned WebView2 SDK;
run `tools/package-nsis.ps1` after setting `WEBVIEW2_SDK_ROOT`. CEF can also be
prepared with `python tools/media-runtime.py`; `--archive` accepts a downloaded
copy of the pinned archive, and `--verify` checks a prepared root.

This source package is not a Windows executable. The Linux preparation environment
cannot establish Windows playback or site/device acceptance; those remain required
CI and device tests. No universal codec, DRM, HEVC, site compatibility or low-memory
performance guarantee is made. Sandbox, site isolation, TLS checks and current
protection remain enabled. License/credit notices accompany the runtime.
