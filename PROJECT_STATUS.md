# Project status — MaenBrowser 1.1.0

This source tree is the expanded Windows desktop development release. It is designed to compile against the pinned CEF/Chromium distribution in GitHub Actions and package an installable Windows build.

Major additions over the foundation: CEF Chrome Runtime integration, persistent local profile/preferences, downloads flow, Windows installer/uninstaller, automated CEF build pipeline, automatic low-memory Lite Mode for <=6 GB systems, MaenBrowser Windows branding/icon resources, and a lightweight MaenBrowser start page. Programmatic unpacked-extension loading is intentionally disabled on the pinned CEF 152 API.

Security boundary: Lite Mode does not turn off sandboxing or site isolation.

Before public production release: run the acceptance/RAM matrix, verify the pinned CEF build and extension APIs, code-sign binaries/installer, configure signed update infrastructure, and perform security/privacy review.

## CI hardening - 2026-10-05
- Pinned GitHub Actions to `windows-2022` because `windows-latest` moved to a VS2026 image that does not provide the requested Visual Studio 17 2022 generator.
- Added an explicit VS2022 C++ toolchain check before CMake configuration.
- Explicitly builds CEF with `USE_SANDBOX=ON`.
- Added `tools/preflight.ps1` to catch repository/version/workflow drift before downloading/building dependencies.
- Chrome Runtime download handling now delegates to Chromium's native download UI; MaenBrowser still observes completion events.


## CI repair 2026-10-05
- Pinned runner: windows-2022 / Visual Studio 2022.
- Replaced Windows tar bzip2 extraction with deterministic two-stage 7-Zip extraction after the previous run stalled until the 90-minute job timeout.
- Exact CEF archive metadata match and SHA-1 verification retained.
- Repository preflight remains enabled before dependency download/build.


## CEF 152 compile repair
- Removed obsolete `CefSettings.chrome_runtime` and `persist_user_preferences` fields.
- Migrated browser preference writes to `CefPreferenceManager`.
- Removed calls to legacy `CefRequestContext` extension APIs absent from CEF 152.
- Added preflight regression guards for those removed APIs.
- Extension programmatic loading is intentionally disabled until implemented through a CEF 152-supported path; no false compatibility claim is made.

## Branding/UI pass
- Added MaenBrowser multi-size Windows icon resource and installer icon.
- Desktop/Start shortcuts explicitly use the application icon.
- Added a lightweight MaenBrowser start page instead of launching directly into Google.
- Window titles retain MaenBrowser identity.
- Kept the proven CEF Chrome Runtime frame for this build rather than risking a wholesale shell rewrite in one release.
- Corrected feature documentation so extension support is not overstated.
