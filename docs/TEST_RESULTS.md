# Test results — development audit batch, 2026-11-05

Baseline `e2692a82b018af5bd34a236787148f9ede49e8f2`, VERSION1.12.2. Changes are a review batch, not a signed production release.

## Executed in this workspace

| Check | Result | Scope |
|---|---|---|
| Baseline `python tools/run-checks.py` | Passed | Portable source/JS/Python and selected C++ tests; Linux, not full Windows |
| Modified `tools/run-checks.py` | Passed with CMake on PATH | Added real CMake ON/OFF updater guard, cohort selection, provenance/scoring and bounded frame/shadow inspection tests; all available portable suites |
| `pwsh -File tools/preflight.ps1` | Passed, existing download review warning | Source/build configuration gate, not runtime safety proof |
| Android `:app:testDebugUnitTest` | Passed4tests,0failures/errors | JVM URL/origin/filter/policy logic |
| Android `:app:lintDebug :app:lintRelease` | Passed;9 warnings per variant,0 errors | Does not prove every runtime API is supported |
| Android `:app:assembleDebug :app:assembleDebugAndroidTest :app:bundleRelease` | BUILD SUCCESSFUL,126tasks; final rerun1m25s | Debug APK, instrumentation APK compiled, unsigned production AAB produced locally |
| New Android external-intent/lifecycle instrumentation | Compiled, NOT executed locally | Requires clean preview install on API29/36 emulator and physical devices; no emulator/KVM available here |
| Idle guard actual headless Chromium DOM | Passed controlled iframe/frame/object/embed/open/closed-shadow and missing-API cases | Visibility, permissions and privileged shadow access were test doubles; not CEF sandbox/extension runtime evidence |
| All-process benchmark PowerShell | Parser checked; no Windows measurement | Runtime process accounting and actual4/8/16GB workload acceptance pending |
| Final source delivery | Passed | Full/changed ZIP integrity, every manifest hash, exact-baseline git apply --check/apply and applied-source package/dataset/idle checks |
| Review exporter | Pinned-base/committed-change/binary fixture test passed | Full-source manifests and patch application checked; fixed initially missing generated manifests in a fresh minimal checkout |
| Engine audit live data | Captured official CEF and active Chrome cohorts | Freshness comparison only; no binary replacement or exploit validation |
| Research database |143canonical requests,155threads; arithmetic/provenance checks passed | Not a representative poll or automated proof of forum statement truth |

The first modified portable invocation failed because `cmake` was installed in the task venv but not on PATH. The command was rerun with the toolchain PATH and passed; no test was skipped to conceal that failure. Android Java/Gradle emit compatibility/deprecation notices; lint succeeds with9 warnings per variant: newer Gradle available(1), JavaScript risk advisory(1), pre-Android12 backup-rule declaration(1), icon shape(1), untranslated/hardcoded strings(5). These are recorded maintenance gaps, not suppressed or represented as a clean warning-free build. allowBackup=false remains in force; JS is needed for web compatibility and native bridges remain absent. Existing Android API29–32 back handling is deliberately retained; do not remove required fallback merely to silence a compile note.

## Independent baseline native CI evidence

[Run42094561247](https://github.com/maenish-ai/MaenBrowser/actions/runs/42094561247), head `e2692a8`, completed successfully; Windows job127481172154 logs were fetched and inspected. Actual assertions included H.264 Main/High and Opus decode, image decode PNG/JPEG/WebP/GIF/BMP/SVG/AVIF, companion service-worker/native API, toolbar popup over ordinary page, Arabic/English settings, Family wrong-PIN/origin rejection and DPAPI persistence, native API rejection from ordinary web pages, encrypted preference restart, and direct GET / POST-popup / redirected-popup attachment files with original request context and content checks. An optional MKV/H.264 path also ran; this does not promise every MKV codec.

These are stronger than a DOM mock, but they test the **baseline**, not this branch's new code, every installed app, physical hardware or authenticated vendor accounts. New branch Windows build/runtime and Android instrumentation remain required. No new call, microphone recording, camera capture, Bluetooth or screen-share test was performed here.

## Acceptance matrix

| Acceptance | Automated/current evidence | Required remaining native/device evidence |
|---|---|---|
|1 Startup/shutdown/restore|Portable saved-session policy; baseline native startup|Crash/process-kill, complete profile upgrade, unsaved-state preservation|
|2 1/5/10/20tabs|Policies only|Windows each memory tier and modes; Android bounded budget explicitly different|
|3 RAM/CPU idle/work|Collector parser|All-process actual samples, repeated workloads|
|4 WhatsApp messages/audio/files|Not executed with real account|Authenticated send/receive/upload/voice and call where supported|
|5 Gmail/Outlook|Not executed with real accounts|Login, compose drafts, upload/download, notification|
|6 YouTube qualities|Local codec fixtures baseline|Real network adaptive1080p/4K, frames/CPU, device decode|
|7 Camera/mic/screen-share|Permission/source controls|Actual two-party call, revoke, device switching, share chooser/audio|
|8 PDF/print|Source paths|Installed viewer, print preview/printer, savePDF|
|9 GET/POST/redirect downloads|Baseline real content assertions passed|Rerun branch, authenticated CDN, cancel/resume, POST error recovery|
|10 Extensions|Companion baseline subset|Third-party package versions/worker/popup/permissions/install/update|
|11 Private browsing|Runtime path expected|Cookie/storage/history cleanup, separate processes, no leaks|
|12 Sandbox retained|CMake/source checks|Candidate sandbox-linked native process verification|
|13 Invalid TLS|Policy source/Androidcancel|Actual expired/self-signed/wrong-host endpoint rejection, no bypass UI regression|
|14 Profile survives upgrade|Not executed here|Signed install migration, encrypted state, interruption and permissions|
|15 Sessions not lost|Portable/deferred snapshot logic|Crash/upgrade, form/frame/shadow/call/download guards on native runtime|
|16 Arabic input|Translations/source tests|RTL/LTR mixed typing, IME, WhatsApp, keyboard/screenreader|
|17 Low resources|Mode logic|4GB actual workload and pressure, no lost drafts/calls|
|18 Physical Android|Local build/lint/unit only|API29/36 and vendor WebView, memory, picker, media, Family and intents|
|19 Install/uninstall|NSIS/Gradle configuration|Fresh/install-over-existing/uninstall without deleting user data unexpectedly|
|20 Signed update/rollback|Unsafe enablement rejected|Full signed metadata/security-floor/transaction protocol not implemented|

## Real-site workflow checklist

All below require official supported web functions; mark unavailable functions N/A with vendor reference, never “passed” from page loading. Use consented test accounts and record app/engine/OS/site date. No cookies/passwords copied from another program and no authentication bypass.

| Site/forum | Login, typing, files, voice, notifications | Camera/mic/call/screenshare if supported | Current branch result |
|---|---|---|---|
|Facebook|Not run|Not run / determine vendor web support|Not tested|
|Instagram|Not run|Not run / determine vendor web support|Not tested|
|Messenger|Not run|Not run / determine vendor web support|Not tested|
|WhatsApp Web|Not run|Not run / determine vendor web support|Not tested|
|Telegram Web|Not run|Not run / determine vendor web support|Not tested|
|Discord|Not run|Not run / determine vendor web support|Not tested|
|Reddit|Not run|Not run / determine vendor web support|Not tested|
|X|Not run|Not run / determine vendor web support|Not tested|
|LinkedIn|Not run|Not run / determine vendor web support|Not tested|
|TikTok|Not run|Not run / determine vendor web support|Not tested|
|YouTube|Not run|Not run / determine vendor web support|Not tested|
|Gmail|Not run|Not run / determine vendor web support|Not tested|
|Outlook|Not run|Not run / determine vendor web support|Not tested|
|Microsoft365|Not run|Not run / determine vendor web support|Not tested|
|Google Docs|Not run|Not run / determine vendor web support|Not tested|
|Google Meet|Not run|Not run / determine vendor web support|Not tested|
|Teams|Not run|Not run / determine vendor web support|Not tested|
|Zoom|Not run|Not run / determine vendor web support|Not tested|
|Slack|Not run|Not run / determine vendor web support|Not tested|
|Twitch|Not run|Not run / determine vendor web support|Not tested|
|Discourse|Not run|Not run / determine vendor web support|Not tested|
|phpBB|Not run|Not run / determine vendor web support|Not tested|
|XenForo|Not run|Not run / determine vendor web support|Not tested|

## Reproduction

`python tools/run-checks.py` with Node, Python, CMake and an available C++ compiler on PATH; `pwsh -File tools/preflight.ps1`; `gradle -p android :app:testDebugUnitTest :app:lintDebug :app:lintRelease :app:assembleDebug :app:assembleDebugAndroidTest :app:bundleRelease` with JDK21/Gradle8.13/SDK36 and existing lock/config. Android instrumented execution uses `:app:connectedDebugAndroidTest` on a clean test installation, not a production user profile. Network/build tools and signing prerequisites are described in the roadmap. No publisher secrets are included.

##Newer delivery

The above is the November5 baseline review. Current1.13.0 results are in [TEST_RESULTS_1.13.0.md](TEST_RESULTS_1.13.0.md); no earlier Windows CI result is reused as proof that new code ran.
