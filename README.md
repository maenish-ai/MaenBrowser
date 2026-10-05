# MaenBrowser 1.5.7 — Unified Lightweight Windows Build

MaenBrowser is a local-first Chromium/CEF browser project focused on a familiar full desktop browsing experience with a conservative low-memory mode for older PCs. It does not require a MaenBrowser cloud account or proprietary sync backend.

## What this build line uses

The Windows application uses C++20 and CEF Chrome Runtime. The build keeps Chromium/CEF for web compatibility while beginning a distinct MaenBrowser identity layer: Windows branding, a MaenBrowser start page and local-first behavior. The current outer browser frame still uses CEF Chrome Runtime for stability. The local profile is stored under `%LOCALAPPDATA%\MaenBrowser`.

## User-facing scope

The source integrates a Chromium browser window, standard navigation/tab behavior exposed by Chrome Runtime, persistent local profile, download interception with Save As and completion actions, local preferences, private-window capability supplied by the runtime, installer/uninstaller, and automated Windows build packaging.

MaenBrowser automatically selects Lite Mode on machines with 6 GB RAM or less. Lite Mode disables speculative prerender and back-forward page caching and caps disk/media caches. It intentionally does **not** disable the Chromium sandbox or site isolation.

## Build

Push the repository to GitHub and run `Build MaenBrowser Windows` in Actions. The workflow fetches the pinned CEF distribution, builds Release x64, stages the Chromium runtime, creates an NSIS installer, and uploads Setup and Portable artifacts.

See `docs/FEATURE_MATRIX.md`, `docs/TEST_PLAN.md`, `docs/SECURITY.md`, and `docs/ARCHITECTURE.md` before calling a release production-ready.

## Important release boundary

This is a serious development release, not a claim of feature-for-feature parity with Chrome, Edge or Firefox. Chrome Web Store compatibility, signed auto-update infrastructure, custom password-manager UI, and measured RAM superiority across all workloads are not claimed until implemented and tested.


## Windows installation

Use the GitHub Actions artifact `MaenBrowser-1.5.7-Windows-x64-Setup` and run `MaenBrowser-1.5.7-Setup.exe`. The installer registers MaenBrowser with Windows Installed Apps, Start Menu, Desktop, App Paths and the Windows browser/default-app capabilities system. See `docs/PLATFORM_SUPPORT.md` for supported-platform boundaries.


## Unified feature direction

1.3 establishes the performance gate and keeps the proven Chromium browser surfaces while the Maen-owned shell is implemented safely. Features that would add disproportionate background RAM/CPU are optional or rejected. The exact implemented-vs-planned boundary is in `docs/FEATURE_MATRIX.md`.
