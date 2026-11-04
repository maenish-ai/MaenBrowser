# Runtime research and provenance (2026-11-04)

## Diagnosis from the user's actual build

- Run: https://github.com/maenish-ai/MaenBrowser/actions/runs/41838904873
- Commit: 24a5c31f3953f9d13997b5044dd21bfaf734753c
- Artifact: MaenBrowser-Windows-x64-Setup-stock-CEF-preview
- Artifact API: https://api.github.com/repos/maenish-ai/MaenBrowser/actions/runs/41838904873/artifacts
- This proves the workflow selected stock CEF; a user's screenshot alone would
  not distinguish a codec failure from a network/player failure.

## Selected ready-built dependency

- Source: https://github.com/psmux/godwinmix-cef/tree/acfb55b74f6b6b9212a94a59d201127cf32390a2
- Release: https://github.com/psmux/godwinmix-cef/releases/tag/cef-154.0.34
- Windows x64 minimal distribution: see config/media-runtime.lock.json for exact
  URL, size and SHA-256, taken from the GitHub release asset digest.
- Upstream states that its builds enable proprietary_codecs=true and
  ffmpeg_branding=Chrome. We inspected its README/build script. We do not execute
  scripts from that repository; we download the pinned distribution archive.
- We verified the downloaded archive's SHA-256 locally. Headers and runtime come
  from this same distribution. Windows compile and playback acceptance remain CI
  responsibilities, not inferred from archive metadata or DLL strings.

## Other sources reviewed

- https://github.com/chromiumembedded/cef/issues/3559 describes the codec build
  gates and existing platform decoder paths. Adding a Windows codec pack alone
  does not enable disabled CEF build gates.
- https://github.com/cefsharp/CefSharp/releases explicitly documents the lack of
  H.264/AAC in default builds; wrapping those builds does not solve this problem.
- https://github.com/winsoft666/cef_binary has old CEF 74/89 artifacts. They were
  rejected for this project because of version and security-age incompatibility.
- godwinmix-cef's CEF 150 release was rejected in favor of its newer CEF 154 build;
  no CEF 152 matching release was found in that provider's release listing.

The upstream scripts use Apache-2.0; the binary includes CEF/Chromium/third-party
notices under their respective terms. Upstream does not supply a codec patent
license. The application publisher retains responsibility for its stated rights.
