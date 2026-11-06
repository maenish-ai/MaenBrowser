# Production release blockers —1.13.0

This is a consolidated source delivery. No test failure is hidden and no unavailable prerequisite is declared complete.

|ID|Blocker|What is ready / what closes it|
|---|---|---|
|RB01 P0|Active codec CEF154 is behind stable CEF156; Chrome cohorts are newer still|Candidate build/provenance/CMake/CTest/runtime pipeline prepared. Need dedicated Windows builder(>=300GBSSD), successful exact-version codec/native security/API tests, advisory/backport review and genuine archive lock promotion. Active lock remains154|
|RB02 P0 distribution|No publisher signing identities/trust roots|Owner provisions Store identity/cert/Android upload key, data-signing public root/feed and optional separate-branch publisher. No private keys committed; executable WinSparkle remains disabled|
|RB03 P0 native acceptance|New Windows DPAPI data transaction/resource stamping has not executed here|Actual branch Windows CEF build and signed-data valid/stale/PIN/failure/restart tests, original-context GET/POST/redirect attachments, TLS/sandbox/profile acceptance|
|RB04 P1 physical performance|No4/8/16GB physical device measurements|Prepared all-process collector/suite/summarizer plus real chat/call/media protocol; execute and retain device/OS/GPU/workload results|
|RB05 P1 Android/device/store|Local APK/AAB/lint/JVM/instrumentation compile is not physical acceptance or Store approval|Execute API29/36 lifecycle/intent/recreation/Keystore/permission/media tests on devices, confirm permanent ID/signing, privacy/screenshots/policy|
|RB06 P1 real compatibility|Third-party extensions, authenticated platforms/calls and assistive technology not certified|Exact package/site/device workflows, no login or site restriction bypass; CEF/WebView ceilings remain|
|RB07 P0 migration/recovery|Legacy installer→MSIX profile behavior and interrupted signed update recovery untested|Installed OS package/identity/profile tests; preserve encrypted state, avoid arbitrary credential copying, only signed repairs above the security floor|

Automatic data protocol is implemented but **unconfigured**; default shipping is not a silently operating publisher service. Store/App Installer update tools are prepared, not an executed/approved production distribution. Native signature verification is in the trusted companion, with separate native origin/PIN/domain/sequence validation; this is not a defense against compromise of that privileged extension or the OS user. No claim all worldwide codecs, extensions, vulnerabilities or websites are covered.

Production tag/signing gates intentionally fail until prerequisites are real; ordinary source/build tests remain available. Do not set acceptance flags just to bypass these checks.
