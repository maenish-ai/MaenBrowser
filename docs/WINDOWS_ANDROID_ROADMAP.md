# Windows / Android phased roadmap

Pinned baseline1.12.2/e2692a8; development branch only. Features remain in place. This is a staged engineering backlog with acceptance gates, not a promise that all features are finished or compatible with every website.

| Phase | This batch | Next small reviewable batch | Exit gate |
|---|---|---|---|
|0 Baseline|Pinned actual commit, separate worktree/branch, inventory, baseline portable checks and read real baseline CI|Keep source/patch/manifests and reproducible test evidence|Baseline identified, no unintended deletion|
|1 Critical fixes|Updater enablement rejection, conservative frame/shadow discard, Android singleTask/external-link tab preservation, engine-freshness cohort correction|Reproduce installed download cancellation, execute new Android lifecycle tests, build supported codec CEF candidate, profile/crash backup|Signed engine/build + download GET/POST/redirect/TLS/Family/no-data-loss acceptance|
|2 Compatibility|Audit actual runtime APIs and media/extension boundaries, honest matrix|Small MV3 fixtures then password/translation/accessibility/productivity extensions, actual account workflows and calls|Native per-package/site evidence with exact versions, no auth bypass|
|3 Performance|All-process collector and4/8/16GB protocol; no numeric claims|Collect real clean/established-profile workloads, event-based scheduling and measured hot-path fixes|Recorded medians/ranges, no security or draft/call regressions|
|4 Community features|143 requests/155 threads, deduplication and transparent score|Prototype highest feasible validated tab/session/accessibility controls; optional heavy functions|Acceptance/rollback for each small change; no inert controls|
|5 Android|Actual Java/Gradle compile/lint/unit/release-bundle build; lifecycle regression instrumentation compiled|Physical low/high-memory devices, Downloads auth context design, notification/screen-share feasibility, reader/accessibility|Device evidence; disclose WebView limitations, no extension parity claims|
|6 Quality/release|Source review bundle, blockers and acceptance plan|Publisher signing, candidate native CI/installer/update transaction, Play metadata/policy and physical tests|No P0 blocker, signed artifacts, source provenance, release notes|

## Architectural rules

Windows stays CEF Chrome Runtime; WebView2 fallback inactive by default and never receives copied login data. Android stays native updatable WebView unless a separately reviewed architecture proves a need to change. Profiles must be directory/request-context isolated. Workspaces/tab groups organize UI and do not imply cookie isolation. Agent mode remains explicit with separate profiles/loopback and Family constraints; Manus/ChatGPT/YourAssistantAgent integration needs each program's actual supported browser connection, not a universal compatibility claim.

Android external URLs now reuse one Activity and open another tab, subject to the existing2/4/6tab budget, instead of replacing the current chat or creating independent policy caches. A full budget refuses visibly and preserves existing tabs. Cold-start incoming links do not overwrite restored tabs; recreation does not replay an old external intent. This does not make Android WebView an authenticated POST download broker; that design must preserve original request context without transferring cookies to another engine.

Sync is optional future E2EE: explicit QR pairing, device keys/revocation, encrypted bookmarks/tabs/settings, conflict handling and recovery. No account requirement, server port exposed to the internet, telemetry or paid service added. Native Firefox/Android Chrome extension support is outside current host capability.

Play prerequisites: final publisher-owned application ID, signing/Play App Signing ownership, target-SDK review, Data Safety/privacy policy, weather/location disclosure, Families/content policy as applicable, accessible UI and real-device testing. Current `ai.maenish.maenbrowser` is provisional; debug `.preview` package is not production. The [target-SDK requirement](https://developer.android.com/google/play/requirements/target-sdk) is checked against capture date, not assumed permanently satisfied by target36.
