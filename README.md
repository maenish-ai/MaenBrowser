# MaenBrowser 1.1.0 — Windows Desktop

MaenBrowser is a local-first Chromium/CEF browser project focused on a familiar full desktop browsing experience with a conservative low-memory mode for older PCs. It does not require a MaenBrowser cloud account or proprietary sync backend.

## What this build line uses

The Windows application uses C++20 and CEF Chrome Runtime. The design deliberately reuses Chromium's mature browser chrome and web platform instead of recreating security-sensitive browser behavior from scratch. The local profile is stored under `%LOCALAPPDATA%\MaenBrowser`.

## User-facing scope

The source integrates a Chromium browser window, standard navigation/tab behavior exposed by Chrome Runtime, persistent local profile, download interception with Save As and completion actions, local preferences, CEF-supported unpacked extensions, private-window capability supplied by the runtime, installer/uninstaller, and automated Windows build packaging.

MaenBrowser automatically selects Lite Mode on machines with 6 GB RAM or less. Lite Mode disables speculative prerender and back-forward page caching and caps disk/media caches. It intentionally does **not** disable the Chromium sandbox or site isolation.

## Build

Push the repository to GitHub and run `Build MaenBrowser Windows` in Actions. The workflow fetches the pinned CEF distribution, builds Release x64, stages the Chromium runtime, creates an NSIS installer, and uploads Setup and Portable artifacts.

See `docs/FEATURE_MATRIX.md`, `docs/TEST_PLAN.md`, `docs/SECURITY.md`, and `docs/ARCHITECTURE.md` before calling a release production-ready.

## Important release boundary

This is a serious development release, not a claim of feature-for-feature parity with Chrome, Edge or Firefox. Chrome Web Store compatibility, signed auto-update infrastructure, custom password-manager UI, and measured RAM superiority across all workloads are not claimed until implemented and tested.
