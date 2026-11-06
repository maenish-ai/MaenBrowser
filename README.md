> 1.13.0 source/build candidate: see [implementation status](docs/IMPLEMENTATION_STATUS.md), [test scopes](docs/TEST_RESULTS_1.13.0.md) and [production blockers](docs/RELEASE_BLOCKERS.md). Active CEF154 is unchanged; newer codec engine and Store signing/device acceptance are not completed in this environment.

# MaenBrowser 1.13.0 — sessions, signed data and Store preparation

Adds bounded session recovery, serialized session edits, original-download recovery, signed protection-data delivery and Store/performance tooling. Native engine, publisher signing and physical-device prerequisites remain explicit.

- [Arabic upload](docs/UPLOAD_1.13.0_AR.md)
- [Changes and required Windows validation](docs/RELEASE_1.13.0.md)

## Previous release context

# MaenBrowser1.12.1

Compact top status strip, automatic permission-aware forecast refresh, native menu branding and separate tabs for browser pages.

- [Arabic upload](docs/UPLOAD_1.12.1_AR.md)
- [Change details and validation limits](docs/RELEASE_1.12.1.md)

## Previous release context

# MaenBrowser 1.12.0

Readable desktop dashboard, local date/time, opt-in licensed MET Norway forecasts, UTF8 installer fixes, same-window new-tab links and updated CI actions.

- [Arabic upload instructions](docs/UPLOAD_1.12.0_AR.md)
- [Changes, validation and limits](docs/RELEASE_1.12.0.md)

## Earlier release context

# MaenBrowser 1.11.0 — release candidate

See [the numbered Arabic requirements and status](docs/release-1.11.0/REQUIREMENTS_AR.md), [actual changes and release blockers](docs/RELEASE_1.11.0.md), and [the new native Android app](android/README.md). Windows still uses the working codec-enabled CEF154 and needs a newer verified engine before claiming current Chromium security fixes. Android artifacts are a QA APK and unsigned AAB, not Play approval.

## Previous 1.10.0 baseline

Builds on the user's successful CEF 154 H.264/AAC release. Adds broader media/image
acceptance coverage, 145,368 local ad domains, two pinned ad-library compatibility
shims, refreshed family data, same-tab ordinary links with OAuth/payment exceptions,
and opt-in YourAssistantAgent integration with isolated cookies and family guards.

- [Arabic upload/build instructions](UPLOAD_AND_RELEASE_AR.md)
- [Release changes and precise limits](docs/RELEASE_1.10.0.md)
- [YourAssistantAgent companion patch instructions](docs/AGENT_COMPATIBILITY_AR.md)
- [Performance scope](docs/PERFORMANCE_1.10.0.md)
- [Pinned codec runtime](config/media-runtime.lock.json)
- [Pinned filter sources](config/filter-sources.lock.json)
- [Pinned compatibility resources](config/ad-shims.lock.json)

Upload extracted source contents including `.github`, `assets/filters` and
`assets/shims`. Push main or run **Build MaenBrowser Windows Setup**. No codec
secrets or self-hosted Chromium builder are required. The Windows workflow keeps
sandboxing, native tests and real playback/image/shim tests; only a successful
build produces Setup. Pushing `v1.11.0` creates a draft prerelease after Windows and Android checks; it does not publish to Play.

Agent compatibility needs BOTH this browser and the separate companion patch in
maenish-ai/YourAssistantAgent, rebuilt and installed there. No third-party agent
is started or given account access by installing the browser. Normal browsing
has no new debugging port. Agent mode is explicit, loopback-only, uses a separate
profile and is unavailable with Family Protection. Shared native settings cannot
be written from agent pages, and parent-policy changes stop agent windows.

Local checks: `python tools/run-checks.py`. Windows build: VS2022, CMake, Node22,
Python3.12+, NSIS and the pinned WebView2 SDK; run `tools/package-nsis.ps1` with
`WEBVIEW2_SDK_ROOT` set. Optional data maintenance uses
`tools/refresh-bundled-filters.py`; it only accepts committed source hashes.
Regenerate the new synthetic fixtures with `tools/generate-media-fixtures.py`
and FFmpeg, never during normal browsing.

The source package is not a Windows executable. New Windows compilation and live
Facebook/Adobe/device acceptance remain required. No universal codec/extension
compatibility, invisible ad blocking, DRM/HEVC support or lightest-browser claim
is made. Engine-internal branding/attribution remain truthful. Separate GPL data
and scripts retain their licenses and source provenance in the package.
