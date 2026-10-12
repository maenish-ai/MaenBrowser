# Validation record — 1.9.6 source candidate

Prepared 2026-10-12 against base `9e407cf3251ac6ffb08923d5808087384dae850d`.

## Executed here
- Package identity/version, referenced assets, immutable bundled filter hashes,
  JS syntax and English/Arabic static-string coverage.
- All 20 `tests/*_test.mjs` files, including settings patches, pressure policy,
  lazy sessions, filter download parsing, state errors, idle rechecks, downloads,
  localization, diagnostics and media-control logic.
- Three Python regression checks (navigation, release invariants and Windows cp1252 encoding simulation).
- Six portable C++20 tests: domains, internal navigation, media policy,
  external-protocol allowlist, native filter snapshot validation and resource selection.
- Workflow YAML parsing, integration-test JavaScript syntax and git diff whitespace check.

See PORTABLE_TEST_LOG.txt for the captured portable runner output. These are logic
and source checks, not proof that the entire Windows application compiles or runs.

## Added but not executable here
Windows browser_smoke.mjs now checks real native revision conflicts, missing
revision rejection, actual restart persistence, lazy family-list loading, existing blocking/PIN/origin checks
and actual playback of the expanded codec fixture matrix. CI requires it before
Setup packaging. The workflow was **not run** from this environment.

## Not verified / external prerequisites
- Full Windows C++/CEF/WebView2 compilation, NSIS install/upgrade and app runtime.
- Actual 2/4 GB RAM performance, GPU/audio devices, permissions/revocation, printing.
- Authenticated website suites, meetings, external apps, DRM and HEVC.
- Repository secret availability or reachability of the publisher's CEF archive.
- Publisher signing certificate, distribution license obligations and newest-engine audit.

No measured speed/RAM saving, universal compatibility or ready-built Setup is claimed.
