# User pain points and engineering response

Community complaints are statements from the sampled users, not independently reproduced defects. Sources are linked; demand strength is per-request in [CSV](COMMUNITY_REQUESTS.csv).

| Pain point | Direct community evidence | MaenBrowser response / limit |
|---|---|---|
| RAM/CPU pressure and unreliable unloading | [Zen8932](https://github.com/zen-browser/desktop/issues/8932), [Zen8822](https://github.com/zen-browser/desktop/issues/8822), [Firefox discussion](https://www.reddit.com/r/firefox/comments/1n6jtyl/) | Keep security in low-resource modes; whole-process benchmark before claims; conservative discard rather than losing work |
| Lost sessions during upgrades/restarts | [Zen11994](https://github.com/zen-browser/desktop/issues/11994), [Brave659931](https://community.brave.app/t/659931) | P0 separate from popularity; test signed upgrade/profile migration, backups, deferred restore; URLs cannot recover unsaved forms |
| Login sessions unexpectedly disappear | [Zen7212](https://github.com/zen-browser/desktop/issues/7212) | Keep request context/profile identity; never transfer cookies to fallback engine |
| Too many tabs and weak organization | [Community wishlist](https://www.reddit.com/r/browsers/comments/1iuwj21/), [Vivaldi hibernation](https://forum.vivaldi.net/topic/111072/hibernate-curren-tab) | Existing manager/saved groups first; native vertical/split/workspace prototypes only after P0 |
| Privacy protections versus site breakage | [Brave/Vivaldi user discussions](https://www.reddit.com/r/vivaldibrowser/comments/1m5krzb/) | Per-site exceptions and diagnostics; no claim filtering is undetectable or all ads disappear |
| Feature bloat and imposed UI changes | [Firefox discussion](https://www.reddit.com/r/firefox/comments/1n6jtyl/), [Vivaldi discussion](https://www.reddit.com/r/vivaldibrowser/comments/1m5krzb/) | Heavy features optional, no extra background service installed in this batch |
| Extension support unclear | [ungoogled3565](https://github.com/ungoogled-software/ungoogled-chromium/issues/3565) | Explicit capability report and real extension acceptance, not a blanket compatible badge |
| Verification/login loops | [ungoogled3226](https://github.com/ungoogled-software/ungoogled-chromium/issues/3226) | Review engine identity/session behavior; do not bypass site authentication or anti-abuse controls |
| Bookmark/history search inadequacy | [HN wishlist](https://news.ycombinator.com/item?id=39247432) | Optional indexed search design; disk/privacy budget and local-only defaults; older single-source evidence gets less demand weight |
| Mobile customization and reader gaps | [Brave Android requests](https://community.brave.app/c/brave-feature-requests/android-requests/65), [Vivaldi mobile](https://forum.vivaldi.net/category/136/mobile-feature-requests) | Respect WebView limits; prioritize downloads, permissions and accessibility over extension parity claims |

Additional user-reported MaenBrowser engineering requirements: attachment downloads canceled after popup changes; confusing installer text; compact clock/weather; internal pages must not replace WhatsApp; native branding consistency. The pinned1.12.2 already fixes the download callback/reparent path and has actual baseline CI attachment tests. These issues still need installed-app/manual acceptance before being declared universally resolved. Native branding must retain mandatory third-party license/Chromium attribution where legally required.
