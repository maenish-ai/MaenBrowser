# Competitor feature comparison — 2026-11-05

Method: source/capability comparison from repository and official vendor documentation, not numeric ratings. Availability can depend on stable rollout, OS, account and policy. **No controlled resource/old-device speed ranking was measured.** “Documented” does not certify every site/codec/extension. Native Chromium competitors share engine foundations but differ in policy, UI and integration.

| Dimension | MaenBrowser | Chrome | Edge | Firefox | Brave | Vivaldi | Opera |
|---|---|---|---|---|---|---|---|
| Resources / old devices | Modes exist;4/8/16GB unmeasured | Memory/Energy Saver documented[1] | Sleeping tabs/efficiency documented[2] | No measurement here | No measurement here | No measurement here | No measurement here |
| Protection | Family, origin guards, sandbox; aging engine release blocker | Chromium security/update model | Chromium plus vendor services | Gecko security/update model | Chromium + Shields | Chromium + native blocker | Chromium + native blocker |
| Privacy | Local controls; weather external request; no new analytics | Vendor settings/services vary | Vendor settings/services vary | Tracking protection | Shields | Blocking/settings | Blocking/settings |
| Tabs | Companion manager/saved sessions; native vertical/split unverified | Groups; split view documented[3] | Vertical/groups/workspaces[4] | Vertical + groups documented[5] | Vertical documented[6]; exact workspaces stable status unverified | Stacks/tiling/workspaces[7] | Islands/workspaces; vertical customization docs[8] |
| Extensions | CompanionMV3 baseline CI; other packages untested | ChromeMV3 | Chromium extensions | Gecko WebExtensions; not all Chrome packages | Chromium compatibility subject API policy | Chromium compatibility subject API policy | Chromium extensions subject policy |
| Accounts / isolation | Separate launch profiles; containers absent | Profiles | Profiles/workspaces not identical to cookie containers | Profiles; containers/channel preview differs[9] | Profiles; container refinements need check | Profiles/workspaces not cookie isolation | Workspaces not automatically cookie isolation |
| Customization | Start page/search/RTL controls; native polish pending | Themes/settings | Sidebar/themes | Themes/sidebar | Themes/verticals | Extensive panels/themes | Sidebar/toolbars |
| Media / calls | Codec fixtures baseline; real calls pending | Build/OS/service dependent | Build/OS/service dependent | Build/OS/service dependent | Build/OS/service dependent | Build/OS/service dependent | Build/OS/service dependent |
| Sync | No E2EE sync transport | Vendor account sync | Vendor account sync | Vendor account sync | Sync chain documented[6] | Vendor sync documented[7] | Vendor sync/flow[8] |
| Android | Native WebView host; no Chrome extensions | Separate mobile app | Separate mobile app | Separate mobile extension policy | Separate mobile app | Separate mobile app | Separate mobile app |
| Accessibility | RTL/UI logic; full assistive-tech acceptance pending | Native/platform support; not tested here | Same evidence limit | Same evidence limit | Same evidence limit | Same evidence limit | Same evidence limit |
| Security updates | Manual releases; secure automatic updater absent, engine behind | Vendor upstream pipeline | Vendor pipeline | Vendor pipeline | Vendor pipeline | Vendor pipeline | Vendor pipeline |
| Developer tools | CEF Chrome Runtime capabilities need installed acceptance | Chromium DevTools | Chromium DevTools | Firefox tools | Chromium tools | Chromium tools | Chromium tools |

## Official references

1. [Chrome performance](https://support.google.com/chrome/answer/12929150), [Chrome tabs/tips](https://www.google.com/chrome/tips/).
2. [Edge performance](https://support.microsoft.com/en-us/edge/learn-about-performance-features-in-microsoft-edge), [SleepingTabs policy](https://learn.microsoft.com/en-us/deployedge/microsoft-edge-browser-policies/sleepingtabsenabled).
3. [Chrome split-view help](https://support.google.com/chrome/answer/16971124), [Chrome productivity release](https://blog.google/products-and-platforms/products/chrome/chrome-productivity-improvements/). Do not use an outdated assumption that Chrome has no split view.
4. [Edge Workspaces](https://support.microsoft.com/en-us/edge/getting-started-with-microsoft-edge-workspaces), [enterprise Workspaces](https://learn.microsoft.com/en-us/deployedge/microsoft-edge-workspaces). Some workspace changes do not imply the whole feature was retired.
5. [Firefox groups](https://blog.mozilla.org/en/firefox/tab-groups-community/), [vertical tabs](https://blog.mozilla.org/en/firefox/vertical-tabs-and-the-firefox-community/), [2025 updates](https://blog.mozilla.org/en/firefox/firefox-2025-updates/).
6. [Brave features](https://support.brave.app/hc/en-us/articles/15872271230221-What-are-all-the-features-Brave-Browser-offers), [vertical tabs](https://brave.com/blog/vertical-tabs/), [sync](https://brave.com/blog/sync-v2/), [desktop releases](https://brave.com/whats-new/desktop/). A feature-request issue is not proof of stable availability.
7. [Vivaldi workspaces](https://help.vivaldi.com/desktop/tabs/workspaces/), [features](https://vivaldi.com/features/workspaces/), [Vivaldi help](https://help.vivaldi.com/).
8. [Opera features](https://help.opera.com/en/latest/features/), [customization](https://help.opera.com/en/latest/customization/), [Tab Islands](https://www.opera.com/features/tab-islands), [Android help](https://help.opera.com/en/mobile/android/).
9. [Firefox containers preview](https://blog.mozilla.org/en/firefox/firefox-containers-preview/) is channel/time-specific, not proof of universal stable Android/desktop parity.

The per-request CSV deliberately says “Not verified for exact requested refinement” where these general documents do not establish equivalence. Future hands-on comparisons must record OS/hardware/version,1/5/10/20 tabs, profile/extension state, workload and method in [benchmark protocol](PERFORMANCE_BENCHMARKS.md).
