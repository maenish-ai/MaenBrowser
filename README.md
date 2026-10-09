# MaenBrowser 1.8.2 — development source

**Status: 1.8.2 source update; Windows build and actual WebView2 playback validation pending.**

Version 1.8.1 passed [Windows CI run 37845006690](https://github.com/maenish-ai/MaenBrowser/actions/runs/37845006690). This update routes direct HTTPS MP4/M4V/M4A/AAC navigation through the existing on-demand Windows media engine and adds a user-invoked video-controls button. See [1.8.2 changes and test boundaries](docs/RELEASE_1.8.2.md).

MaenBrowser uses C++20, CEF Chrome Runtime and an embedded WebView2 surface for WhatsApp and direct media files. This development branch adds a bundled Manifest V3 control panel backed by native request filtering. It keeps Chromium sandboxing and site isolation enabled.

## Changes in this source

- Toolbar advertising-domain blocker with on/off and per-site exceptions.
- PIN-protected family settings: allowlisted sites (default family policy), explicit blocked domains, adult/violence domain lists and supported search-engine SafeSearch parameters.
- Local protected settings using Windows DPAPI and PBKDF2 PIN verification.
- Configurable download folder and ask-before-saving in CEF and embedded WebView2.
- Conservative idle-tab discarding, tab search, pinning, grouping and explicitly saved local sessions.
- Download controls, plain-text reader and local media capability checks.
- WebView2 origin checks, permission prompts, navigation filtering and event-driven resizing.

Read [the delivery report](docs/DELIVERY_1.8.0.md) before running or distributing this version. Older documents, including [the previous README](docs/README_HISTORY.md), describe earlier versions and are not proof that new features have passed testing.

## Build and validation

On Windows, run the **Build MaenBrowser Windows Setup** GitHub Actions workflow. It runs static checks, builds with Visual Studio 2022, runs native and browser integration tests, then packages Setup using NSIS. The workflow is configured for main and development/** branches, and can also be started manually. A failed gate must be fixed before release.

Local checks completed during development:

```sh
python tools/verify-package.py
node tests/tab_policy_test.mjs
g++ -std=c++20 -I. tests/domain_rules_test.cpp -o domain-tests
./domain-tests
```

Version 1.8.1 completed run 37845006690 successfully. The new 1.8.2 changes have not yet run on Windows. Local checks do not establish a measured performance improvement or complete third-party translation.

## Compatibility boundaries

There is no claim of being the world's lightest browser, universal extension compatibility, complete ad removal, perfect parental filtering or universal media playback. Codec/DRM support depends on the shipped runtime and applicable licensing. Family protection is a browser feature, not a substitute for a separate restricted Windows child account. Filters are bundled snapshots; they are not currently refreshed in the background.
