# Test results — MaenBrowser 1.13.0

Linux source/build-candidate validation on2026-11-06. No GitHub write or new remote CI run. Read-only remote-main verification returned the exact pinned baseline e2692a82b018af5bd34a236787148f9ede49e8f2. [Machine-readable result](research/2026-11-06/validation-1.13.0.json).

| Check | Actual result | Limit |
|---|---|---|
| Portable checks | Passed33JS suites,22Python checks including package validation,13portable C++ test programs | Not a Windows browser build |
| Real cryptographic tests | RSA-PSS signatures generated and verified; tamper/replay/expiry/app-floor/origin/hash checks passed | Test keys ephemeral; production publisher key absent |
| Three-list data transaction client | Parent-lock enforcement, failed native commit preservation, acknowledged current no-op and lost-ack reconciliation passed | Transport/native API controlled test doubles; Windows DPAPI execution pending |
| Native policy helpers | Domain/sequence validation and parent policy PIN rules passed in actual portable C++ programs | Windows endpoint/runtime integration assertions prepared, not executed |
| Sessions | Concurrent operations serialized, failed writes/quota preserved previous set, damaged-primary recovery, property-order-independent checksum, private/token/deferred handling, lazy duplicate-safe restore passed | Does not reconstruct forms/POST/calls; actual crash/OS-upgrade acceptance pending |
| Download recovery | Original download ID resume only; fresh resumability check; no replacement URL request; safety/error advice passed | Real branch downloads require Windows, though baseline native GET/POST/redirect tests exist |
| Actual Chromium DOM UI | English/Arabic search/pagination, settings search keeping PIN visible, native-connection UI, unconfigured signing state and download UI passed without page errors | Chrome/native APIs mocked; not a CEF acceptance run |
| Package/source verification | Passedincluding native version alignment and complete static EN/AR control translations | No new native exe produced here |
| PowerShell | All scripts parser checked; source preflight passed | MSIX/resource stamping/candidate/benchmark Windows execution pending |
| Workflow YAML | All workflow files parsed | No Actions dispatch or remote execution performed |
| Android | JDK21/Gradle8.13/SDK36: unit tests4passed, debug/release lint0errors, APK/instrumentation APK and unsigned AAB built; shared parental state, ordered writer and live policy observer compiled; final run126tasks |8lint warnings per variant; instrumentation compiled, not run on emulator/device |
| Upstream intelligence | Official engine capture,126browser-related historical CISA KEV entries | Not an exhaustive CVE list or a vulnerability verdict for this app |
| Performance | Summarizer calculations tested; Windows1/5/10/20runner syntax and collector parser checked | No4/8/16GB physical RAM/CPU/latency figures were generated |
| Production readiness | Correctly reports NOT ready | Old active engine, publisher keys/identities, native/physical/Store acceptance missing |

Android8warnings: newer Gradle available(1), JavaScript enabling advisory(1; web compatibility needs JS and no generic native bridge is added), icon shape(1), hardcoded/untranslated native strings(5). Pre-Android12 backup declaration warning was resolved. Java/Gradle compatibility/deprecation notices remain; no suppression to fabricate a warning-free result.

Initial development checks exposed a Node test syntax/crypto-global setup issue, missing new static translations and an obsolete exact source-call-count assertion. These were corrected; final suites pass. They were not hidden by skipping tests. Signature tests use actual cryptographic operations; storage/transport doubles remain labeled as such.

## Mandatory remaining acceptance

The1.12.2 baseline [Windows run42094561247](https://github.com/maenish-ai/MaenBrowser/actions/runs/42094561247) proved specific real codec/image, native origin/PIN/DPAPI and GET/POST/redirect attachment cases. It does not prove this new source. Expanded browser_smoke now asserts native atomic security data, stale/invalid rejection and parent-policy PIN protection while Family is off. Candidate workflow includes exact-engine provenance, sandbox, CMake/CTest and actual browser runtime integration; it has not run here.

Still required: actual branch Windows compile/runtime; unchanged original request context for authenticated/POST/redirect downloads; invalid TLS rejection; actual sandbox/process state; corrupted/protected data and upgrade interruption; legacy→MSIX profile retention; real WhatsApp/Gmail/Outlook/Meet/Teams/Zoom and other authorized platform workflows; real camera/mic/screen-share/Bluetooth; actual third-party extension/assistive-tech versions; physical Android29/36; and4/8/16GB all-process workloads. An installed page opening or a simulated permission test is not a completed call.

No physical benchmark, signed executable upgrade/rollback, Store submission, CEF156 binary build, Windows installer build or universal codec/extension certification is claimed. See [blockers](RELEASE_BLOCKERS.md).

Android delivery includes compiled shared parental state, a single ordered idle-retiring session writer and live-Activity policy observers; device execution of that race/lifecycle check remains pending. Debug APK v2 signature was verified; release AAB was verified unsigned.

Source delivery ZIPs, source manifests, exact-baseline patch application and applied-source checks were verified.
