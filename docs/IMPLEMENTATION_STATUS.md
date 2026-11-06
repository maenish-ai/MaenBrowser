# MaenBrowser 1.13.0 — consolidated delivery

Date2026-11-06. One review/source delivery, based on origin commit `e2692a82b018af5bd34a236787148f9ede49e8f2` plus the prior audit commits through `7b97c60`. Working branch `development/1.13.0-complete`. No GitHub push, branch creation, release, workflow dispatch or repository-setting change was performed for this task, as requested. Earlier failed push attempts belong to the [November5 audit](IMPLEMENTATION_STATUS_2026-11-05.md).

**This is a source/build candidate, not a claim that all production prerequisites are complete.** The active binary engine lock remains codec-enabled CEF154; a newer codec binary cannot be built in this Linux environment with2.7GB free disk. The current CEF156 source candidate has a real isolated build/runtime-test path, but has not been built or executed here. Publisher-owned Store identities/signing keys and physical4/8/16GB acceptance are not available. Production publication is gated rather than inventing successful results.

## Implemented in this delivery

| Area | Concrete implementation | Evidence / boundary |
|---|---|---|
| Engine upgrade tooling | Explicit separate runtime lock, candidate archive/layout/provenance verification, CMake sandbox build, CTest and actual browser-smoke path on dedicated Windows builder | `tools/test-cef-candidate.ps1`, expanded candidate workflow. Active lock never falsely changed to an unavailable DLL |
| Security intelligence | Current official engine snapshot plus126 historical browser-related CISA KEV records, vendor/product filtering | This is a coverage subset, not proof these CVEs affect MaenBrowser or that all were fixed |
| Parent policy | Existing parent PIN now protects allow/block lists, category/SafeSearch and allow-only changes even while Family is off; ordinary ad/download changes remain available while off | Portable policy tests; actual Windows endpoint acceptance pending |
| Signed data transport | RSA-PSS/SHA256, app-version floor, sequence/expiry/origin/size/hash/domain validation | Actual crypto and controlled transport tests; publisher key intentionally unconfigured |
| Native security-data persistence | One DPAPI-protected atomic3-list bundle +backup, native replay floor and parent-PIN checks; union with bundled Family categories | Windows integration assertions prepared, not executed here; trusted companion is part of the trust boundary |
| Background updates | Daily optional client check; parent-required notification; Actions prepares signed artifacts on reviewed filter changes/weekly/manual, optional separate data-branch publisher | Default no secret/key/publication variable means no feed publication. No executable download/install through JS |
| Application updates | MSIX packaging +signed App Installer generation, Store-managed update route; legacy unsigned WinSparkle path still blocked | Real publisher identity/certificate, current engine evidence and acceptance required before production signing |
| Sessions |3bounded local automatic URL checkpoints, damaged-entry fallback, known token/private/internal exclusions, opt-out/delete, deferred-page decoding | SHA256 is corruption detection, not encryption; URL-only restore cannot recover drafts/calls/POST |
| Manual session consistency | Serialized worker mutations, bounded atomic current/previous sets, corruption recovery; retry restore skips existing live/deferred addresses | Real storage failure/concurrency tests; does not reconstruct unsaved DOM |
| Tabs / resources | Search/scope/recent/title sorting,100rows initially and capped pagination, protected idle discard retained | Reduces manager DOM work, not an unmeasured claim about total RAM |
| Downloads | Resume original download identity with a fresh state check; actionable network/auth/disk/security advice; no URL/POST replay | Existing GET/POST/redirect native path preserved; new branch Windows acceptance pending |
| Settings | Local settings search keeps PIN controls visible, new AR/EN strings, honest resource-mode labels, signed-update status and opt-out | Actual DOM QA with mocked native/Chrome APIs, not real CEF certification |
| Identity / version | Fixed stale native1.10version string, generated Windows VERSIONINFO, bootstrap identity stamping before native acceptance, synchronized1.13.0 metadata | Native resource stamping parser checked; Windows execution pending |
| Android | Preserve selected tab/address handoff across Activity recreation without plaintext URLs in saved Bundle; encrypted durable store retained; backup-rule correction; publisher-ID mapping and derived versionCode11300 | APK/AAB/instrumentation compile, JVM tests and lint run; physical lifecycle execution still needed |
| Performance tooling | Optional selected-root all-process capture, actual-Windows synthetic1/5/10/20tab runner, range/median summarizer | Parser/unit evidence only; no fabricated4/8/16GB numbers |
| Store preparation | Windows manifest/assets/SDK pack/sign tool, App Installer, Android publisher-ID/signing metadata checks, store checklist | Unsigned AAB is not Play submission-ready; Windows MSIX has not been built here |

## Preserved boundaries

Family Protection stays enabled/authoritative where selected. No Sandbox/Site Isolation/TLS weakening, arbitrary website-to-OS bridge, public debug server by default, copied cookies/tokens across engines, new telemetry, paid service or unsupported universal codec/extension claim. New benchmark endpoints are explicit, loopback-only, isolated-profile test processes. Normal browsing still uses CEF; WebView2 routing stays off by default.

See [complete change notes](RELEASE_1.13.0.md), [test results](TEST_RESULTS_1.13.0.md), [security-data protocol](SIGNED_SECURITY_DATA.md), [Store preparation](STORE_DISTRIBUTION.md), [performance protocol](PERFORMANCE_1.13.0.md) and [current release blockers](RELEASE_BLOCKERS.md).

Android recreation safety: production Activities now share one application-context encrypted-state object and one ordered, idle-retiring session writer. Policy listeners enforce parent changes in the live Activity, including cancellation of media/file grants and blanking existing documents; retired Activities unregister. This prevents a late session write from persisting an old parent-policy copy. Instrumentation asserts shared identity, but actual device execution remains pending.
