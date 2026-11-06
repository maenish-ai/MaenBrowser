# Extension compatibility

`src/extensions/extension_manager.cpp` is a compatibility facade. LoadUnpacked=false, Unload=false and LoadAll=0 intentionally avoid removed public CEF extension APIs after Chrome Runtime changes. Returning false is an honest unsupported operation, not proof of an installed manager. Do not reintroduce removed CefRequestContext::LoadExtension calls.

App startup uses Chrome Runtime and `--load-extension` for the internal companion. GetDefaultClient supplies the application's browser client for runtime-created tabs. Baseline [native CI](https://github.com/maenish-ai/MaenBrowser/actions/runs/42094561247) proves a companion MV3 service worker/native API and toolbar popup on an ordinary page for the pinned engine; it does not prove arbitrary Chrome Web Store packages, every extension API, updates or Firefox extensions.

| Test target | Expected surface | Evidence status |
|---|---|---|
| Bundled companion MV3 | worker, popup, tabs/storage/scripting/native control origin | Baseline Windows CI subset passed; modified idle probe needs fresh Windows run |
| Chrome runtime extension manager | enable/disable/uninstall/permissions/unpacked installation | Manual native acceptance pending; facade is not this UI |
| Bitwarden or KeePassXC integration | password forms, worker, popup; native messaging if applicable | Not installed/tested; publisher-approved package + native-host support review required |
| Translation extension | selection/context menu/content script and language UI | Not installed/tested |
| Accessibility extension | keyboard focus, page DOM, permissions | Not installed/tested; test with screen reader, not DOM mock alone |
| Notes/productivity extension | popup, background worker lifecycle, storage/export | Not installed/tested |
| MV3 declarativeNetRequest blocker | declarative rules/quotas/host permissions | Not installed/tested; no MV2 API restoration workaround |
| Firefox WebExtensions package | Different signing/API host | No supported Firefox extension host |
| Android Chromium extensions | WebView lacks extension runtime | Unsupported in current architecture |

## Practical design

Use supported Chrome Runtime UI/APIs, not an imitation manager with inert install buttons. Establish a versioned capability report for worker lifecycle, action popup, options page, tabs/groups/storage, scripting isolated world, permissions, DNR, downloads, webRequest limitations, nativeMessaging and management. Deny or visibly mark unsupported operations. Keep companion separate from user packages and preserve trusted-origin boundaries. Per-extension permissions and update provenance must be reviewable; never silently grant all sites or fetch executable packages from arbitrary pages.

Build reproducible small MV3 fixture extensions (service-worker suspension/wake, action popup, optional host grant/revoke, DNR rule, file download) and test the installed browser on the candidate CEF engine. Then test independently distributed real extensions under separate test profiles; record version/hash/license, prompts, errors and successful user workflows. A fixture does not certify Bitwarden or a real-world call. Native Chrome runtime compatibility remains the ceiling, and policy/store/native-host restrictions can differ.

References: [MV3 migration](https://developer.chrome.com/docs/extensions/develop/migrate), [permissions](https://developer.chrome.com/docs/extensions/develop/concepts/declare-permissions), [CEF branches](https://github.com/chromiumembedded/cef/blob/master/docs/branches_and_building.md), [security](https://chromium.googlesource.com/chromium/src/+/main/extensions/docs/security_faq.md).
