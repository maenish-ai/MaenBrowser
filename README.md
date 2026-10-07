# MaenBrowser 1.5.25 — Unified Lightweight Windows Build

MaenBrowser is a local-first Chromium/CEF browser project focused on a familiar full desktop browsing experience with a conservative low-memory mode for older PCs. It does not require a MaenBrowser cloud account or proprietary sync backend.

## What this build line uses

The Windows application uses C++20 and CEF Chrome Runtime. The build keeps Chromium/CEF for web compatibility while beginning a distinct MaenBrowser identity layer: Windows branding, a MaenBrowser start page and local-first behavior. The current outer browser frame still uses CEF Chrome Runtime for stability. The local profile is stored under `%LOCALAPPDATA%\MaenBrowser`.

## User-facing scope

The source integrates a Chromium browser window, standard navigation/tab behavior exposed by Chrome Runtime, persistent local profile, download interception with Save As and completion actions, local preferences, private-window capability supplied by the runtime, installer/uninstaller, and automated Windows build packaging.

MaenBrowser automatically selects Lite Mode on machines with 6 GB RAM or less. Lite Mode disables speculative prerender and back-forward page caching and caps disk/media caches. It intentionally does **not** disable the Chromium sandbox or site isolation.

## Build

Push the repository to GitHub and run `Build MaenBrowser Windows` in Actions. The workflow fetches the pinned CEF distribution, builds Release x64, stages the Chromium runtime, creates an NSIS installer and uploads the Setup artifact only.

See `docs/FEATURE_MATRIX.md`, `docs/TEST_PLAN.md`, `docs/SECURITY.md`, and `docs/ARCHITECTURE.md` before calling a release production-ready.

## Important release boundary

This is a serious development release, not a claim of feature-for-feature parity with Chrome, Edge or Firefox. Chrome Web Store compatibility, signed auto-update infrastructure, custom password-manager UI, and measured RAM superiority across all workloads are not claimed until implemented and tested.


## Windows installation

Use the GitHub Actions artifact `MaenBrowser-1.5.25-Windows-x64-Setup` and run `MaenBrowser-1.5.25-Setup.exe`. The installer registers MaenBrowser with Windows Installed Apps, Start Menu, Desktop, App Paths and the Windows browser/default-app capabilities system. See `docs/PLATFORM_SUPPORT.md` for supported-platform boundaries.


## Unified feature direction

1.3 establishes the performance gate and keeps the proven Chromium browser surfaces while the Maen-owned shell is implemented safely. Features that would add disproportionate background RAM/CPU are optional or rejected. The exact implemented-vs-planned boundary is in `docs/FEATURE_MATRIX.md`.


## 1.5.25 adaptive performance/media pass

Low-memory cache ceilings are now tighter on 4 GiB systems while Chromium GPU/WebGL/video acceleration remains available. The start page includes a local Media & 3D capability check for current codec/API claims. See `docs/PERFORMANCE.md`. This does not claim that unsupported proprietary codecs or 4K decoding can be made fast in software on hardware that lacks the required capability.


## 1.5.25 download reliability
MaenBrowser explicitly continues user-initiated downloads through the CEF callback, including blob/service-worker media downloads used by modern web apps. Save As remains user-controlled. Media playback codec availability is tested separately and is not falsely inferred from download support.

### 1.5.25 CI/download guard repair
This revision corrects the 1.5.12 preflight false positive while retaining explicit CEF download continuation for WhatsApp/blob/service-worker downloads. It does not claim that H.264/AAC playback is solved by this change; playback codec support remains a separate CEF build/licensing boundary.


## 1.5.25 media release gate
- Treats WhatsApp/Web MP4 playback as a release blocker rather than a cosmetic issue.
- Adds a documented compatibility matrix for H.264/AAC, VP8/VP9, AV1, Opus/Vorbis, MP3/FLAC, MSE, WebRTC and WebCodecs.
- Preserves GPU/hardware decode and the adaptive low-memory policy; no external codec pack or background media service is added.
- Adds CI runtime-integrity checks and keeps Setup-only artifact publishing.
- Proprietary-codec redistribution remains gated on a version-matched custom CEF build plus licensing review; the public CEF binary is not falsely advertised as full H.264/AAC support.


## 1.5.25 CI + media-runtime correction
- Removes the hard-coded 1.5.14 GitHub Actions version check that caused run 37429335016 to stop before preflight/build.
- VERSION is now the single release-version source checked against CMake, the version header and NSIS, so the same drift cannot silently recur.
- Setup-only publishing remains intact.
- Preserves Chromium GPU acceleration, hardware video decode, MediaSource, WebRTC, WebCodecs, VP8/VP9, AV1 and standard audio paths.
- Adds an optional SHA-256-pinned custom CEF archive path (`MAEN_CEF_ARCHIVE_URL` + `MAEN_CEF_ARCHIVE_SHA256`) for a reviewed, version-matched media-enabled CEF runtime.
- The official CEF fallback remains available for normal CI; it is not falsely labeled as full H.264/AAC support.
- No random codec DLL, codec pack, TLS bypass, sandbox weakening or forced GPU override is introduced.


## 1.5.25 custom CEF media build path
- Added `tools/build-cef-media.ps1` with the Chromium/CEF media build flags `proprietary_codecs=true ffmpeg_branding=Chrome`.
- Added `docs/CUSTOM_CEF_MEDIA_BUILD.md` with version/provenance, WhatsApp runtime acceptance tests, and licensing boundary.
- Normal app CI remains lightweight; it consumes a reviewed custom CEF archive only when URL + SHA-256 are explicitly configured.
- This source change alone does not claim WhatsApp H.264 playback until the custom runtime is actually built, integrated and runtime-tested.

### Media-enabled release CI (1.5.25)
A normal push always validates the project. The Setup job is intentionally conditional on `MAEN_CEF_ARCHIVE_URL` and `MAEN_CEF_ARCHIVE_SHA256`; missing values skip the release job instead of failing the whole workflow. This prevents wasting Actions minutes while also preventing a stock-CEF Setup from being mistaken for the WhatsApp-video build.

## WhatsApp media on Windows (1.6.1)
WhatsApp Web is routed to MaenBrowser's native `MaenMediaHost.exe`, powered by the Microsoft Edge WebView2 Evergreen Runtime. This avoids requiring a custom 100+ GB Chromium/CEF build just to obtain H.264/AAC support. Other browsing remains on the pinned CEF 152 engine.
