# MaenBrowser 1.8.0 — development source

**Status: FIX-02 development source. Windows compilation and native tests passed in CI; browser integration stopped at the ad-block probe. That probe is corrected here and awaits a new Windows run. No new Setup has been built.**

See [FIX-02 status and upload instructions](docs/FIX_02_STATUS.md) for the latest verified result.

MaenBrowser uses C++20, CEF Chrome Runtime and an embedded WebView2 surface for WhatsApp. This development branch adds a bundled Manifest V3 control panel backed by native request filtering. It keeps Chromium sandboxing and site isolation enabled.

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

Windows C++ compilation, native tests, extension loading and the native settings connection passed in GitHub run 37837041845. Full browser integration has not passed yet; the corrected probe in this package still requires a new Windows run.

## Compatibility boundaries

There is no claim of being the world's lightest browser, universal extension compatibility, complete ad removal, perfect parental filtering or universal media playback. Codec/DRM support depends on the shipped runtime and applicable licensing. Family protection is a browser feature, not a substitute for a separate restricted Windows child account. Filters are bundled snapshots; they are not currently refreshed in the background.
