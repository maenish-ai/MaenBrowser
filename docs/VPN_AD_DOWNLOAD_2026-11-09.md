# VPN, ad filters and download safety — 2026-11-09

Base: GitHub `b7416ccf1eac5910cd4279dab52f135f376c0474`. Existing normal build for this baseline succeeded on Windows and Android; its Windows fixture reports direct, POST and redirected downloads retained their request context. Fixture success is not a live GitHub/Gmail/WhatsApp/Turbo VPN test.

## VPN diagnostics
The popup's **Check recent network error** button reads one recent main-frame error category from the native local endpoint. Categories are network changed, offline, DNS, proxy/tunnel, timeout, certificate or other. Codes are classified against Chromium's net error list. Only the last category and monotonic timestamp are held in memory until replaced or browser exit, and are exposed for at most five minutes; there is no URL, hostname, address, VPN identity or history in this feature. Private-profile events, aborts and client-blocked requests are excluded. This is browser-wide across normal tabs, not proof that Turbo VPN caused a failure. Chromium's normal error page and traffic handling remain unchanged; no requests are replayed, no system proxy/VPN/DNS settings are modified, and TLS checks remain enabled. Some sites display their own offline messages without a Chromium load error, which this diagnostic cannot observe.

Turbo VPN could not be installed or tested in this Linux environment. To diagnose, on Windows connect Turbo VPN, reproduce the issue, open the shield and press the diagnostic button within five minutes. Compare after disconnecting and reconnecting; note whether error follows VPN server/location, DNS, proxy/tunnel or site. Do not share sensitive URLs or credentials. Do not disable TLS or route traffic outside the VPN to test.

## Ads
The existing pinned HaGeZi GPL-3.0 multi-list supplement adds 32 domain entries to ads-extra (some already exist in another list), selected by documented advertising prefixes and reproduced with the existing hash-pinned refresh script. Domain filtering can block legitimate content; the user-controlled site exception remains available. Existing local compatibility shims continue without impression requests. No simulated clicks, fake human signals or guarantee against detection is provided.

## Downloads
The original CEF request and lifetime remain intact. If another tab appears inside a popup hidden for a download, its window is restored so the tab cannot become inaccessible. This does not promise that every page, authentication popup, blob, Gmail/WhatsApp link or cross-origin workflow avoids a new tab. No automatic browser or download cancellation is added.

## Validation
Portable JS/Python/C++ suite and PowerShell preflight passed locally. Network-category/UI unit checks and source lifetime tests passed. The changed Windows binary and the changed integration smoke were not run in this environment. Hardware, live sites and Turbo VPN acceptance remain unverified.
