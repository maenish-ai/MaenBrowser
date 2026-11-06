# MaenBrowser current-state audit — 2026-11-05

Baseline: `e2692a82b018af5bd34a236787148f9ede49e8f2`, origin/main, VERSION **1.12.2**. Development branch: `development/community-audit-2026`. This is an incremental audit and P0 review batch, not a newly released browser. No main merge, repository setting change, new telemetry or public network listener.

## Evidence and scope

The [baseline inventory](research/2026-11-05/source-inventory.csv) records all 430 tracked baseline files and SHA256. Source review targeted CMake, runtime locks, app lifecycle, extensions, protection, storage, media, updater, companion, Android, installers, tools, workflows and tests; inventory coverage does not mean every line was manually audited. Portable tests exercise logic and selected C++ components, not a complete Windows browser. No native Windows session or physical Android device is available in this Linux workspace.

An independently fetched, completed [baseline Windows/Android CI run](https://github.com/maenish-ai/MaenBrowser/actions/runs/42094561247), head `e2692a8`, provides actual Windows runtime evidence for specific cases. It predates this branch. The repository's older `VALIDATION_1.12.2.json` correctly said Windows had not then been run locally; the subsequent remote CI result adds evidence rather than changing that historical claim.

Classification: **tested** means only the identified test scope; **needs runtime test** is source present without current native acceptance; **partial** lacks part of the requested behavior; **absent** no identified implementation; **not possible on current engine** requires a different host capability; **restricted** needs security/legal prerequisites. A capability can be tested at unit level and still need device acceptance.

| Component / capability | Classification | Evidence / remaining gap |
|---|---|---|
| Windows CEF Chrome Runtime | Present, baseline native CI tested | `CMakeLists.txt`, `src/app/maen_app.cpp`; CEF154/Chromium154.0.8037.98 pinned, security freshness blocked |
| Codec distribution lock | Present, integrity checks tested | `config/media-runtime.lock.json`; SHA256 locked; this is not publisher authenticity or independent source reproducibility |
| CEF156 upgrade candidate | Partial, unbuilt | `config/cef-upgrade-candidate.json`; official CEF stable at capture, not all Chrome stable cohorts; codec build/ABI/native tests required |
| Sandbox / TLS / process mitigations | Present, portable policy checks; baseline CI subset | `src/security/windows_hardening.cpp`, CMake; hardware exploit resistance not established by flags |
| Family Protection / native control origin | Present, baseline native CI tested subset | `src/protection/`, DPAPI local state, PIN, exact trusted extension origin, fail-closed damaged state; category false negatives and bypass-resistance still need ongoing tests |
| Native ad/tracker filtering + exceptions | Present, partial real-web coverage | Native protection + companion; no guarantee a website cannot detect filtering |
| Filter updates | Partial, review required | `assets/companion/filter-update.js`; rule data only; no remote arbitrary executable scriptlets added |
| Extension LoadUnpacked / Unload / LoadAll facade | Partial intentionally unsupported | `src/extensions/extension_manager.cpp`: returns false/0 for removed legacy CEF API; not an implemented extension manager |
| Chrome Runtime companion MV3 worker/popup | Present, baseline native CI tested subset | Companion worker/native API and ordinary-page toolbar popup exercised; startup `--load-extension`, not legacy API |
| Arbitrary Chrome Store extensions / updates | Needs runtime tests, not certified | Runtime may expose native manager; no bundled third-party acceptance evidence; no promise all Chromium extensions |
| Firefox extension host | Not possible with existing Chromium extension engine | Different APIs/distribution model; no host implementation |
| WebView2 optional fallback | Present, inactive by default | `src/media/media_policy.h`: embedded routing false; separate profile and permissions; no normal double-rendering or session copying |
| CEF download context and popup attachments | Present, baseline native CI tested | Actual direct GET, POST popup body/referrer and redirect attachment content assertions; not every authenticated site |
| Full download queue/resume/integrity UI | Partial | Chromium/download callbacks and companion UI; user-facing complete queue and arbitrary POST resume not proven |
| Resource modes Automatic/UltraLite/Lite/Balanced/Performance | Present, logic tested | `src/app/resource_mode.cpp`, companion memory policy; no 4/8/16GB device benchmark yet |
| Conservative idle discard | Present, portable tests | Protects active tabs, editors, calls, forms; this branch protects all frames, shadow trees and uncertain inspection; Windows API availability untested |
| Saved sessions/deferred restore | Partial, portable tests | `session-restore.js`, `tabs.js`; URL/group snapshots cannot reconstruct unsaved DOM, arbitrary POST or calls |
| Native crash restore preferences | Partial | `browser_preferences.cpp` sets restore preference on startup; review interaction with user preference and companion restore before changing |
| Vertical tabs / nested stacks / native split rendering | Not verified / absent native implementation | Companion tab organizer is not native tab strip or multi-pane rendering |
| Workspaces | Partial | Saved sessions/groups; not verified native isolated workspace containers |
| Multiple profiles and opt-in agent CDP | Partial, safety logic tested | Separate profile directories, loopback agent endpoint, Family restrictions; external agent/vendor handshake not tested |
| Automatic website container rules | Absent | Needs explicit request-context partitioning; never copy existing cookies/tokens |
| Start page, search, clock, weather | Present, tests plus visual acceptance needed | `start.html`, `start-dashboard.js`, `weather-policy.js`; device clock/timezone, saved weather opt-out, cached MET Norway coarse location; external weather is not instantaneous sensor data |
| Branding and internal links | Partial | Resource-ID overrides and internal-navigation policy; full locale/resource-pack and background-tab selection acceptance pending |
| Reader / Arabic RTL / high contrast | Partial, portable tests | Reader sanitization and companion UI; keyboard/screen-reader/native RTL manual acceptance pending |
| Automatic signed updater | Absent usable secure pipeline; enabling restricted | Off by default, no configured trusted signed feed, security floor or transaction protocol. This branch rejects ON rather than shipping insecure updater |
| Android WebView app | Present, local compile/lint/unit tests | `android/app/src/main/`; updatable platform WebView, target36/min29, separate architecture from CEF |
| Android tabs/uploads/downloads | Partial | Bounded tabs, Android picker, DownloadManager; authenticated POST/blob downloads, notification and screen-sharing host gaps |
| Android Family state | Present, unit/build evidence; device pending | AES-GCM AndroidKeyStore, PIN derivation/rate limits; singleTask in this branch prevents normal duplicate-Activity policy caches; external links preserve existing tabs |
| Android Chrome extensions | Not possible in Android WebView host | No extension runtime; do not advertise desktop parity |
| Desktop/Android E2EE sync | Absent | Local export/import is not a transport; needs pairing/recovery/key design |
| Android Play production distribution | Partial / release blocked | Unsigned release AAB can build; publisher-owned signing, final application ID, disclosures and device acceptance required |
| HEVC / DRM / legacy Adobe plugins | Restricted / unsupported paths | Platform/licensing/CDM agreements and vendor APIs; Flash/NPAPI not restored; browser web Adobe/Office apps require functional tests |

The screenshots showing Node20/upload-artifact v5 warnings refer to older workflow revisions. This pinned baseline already uses upload-artifact v7 and newer setup actions, and its inspected CI run succeeded; warnings are distinguished from failed jobs rather than suppressed with environment flags.

See [security](SECURITY_AUDIT.md), [engine plan](ENGINE_UPGRADE_PLAN.md), [tests](TEST_RESULTS.md), [release blockers](RELEASE_BLOCKERS.md) and [full gap matrix](FEATURE_GAP_MATRIX.md). Existing features and Family Protection are preserved.
