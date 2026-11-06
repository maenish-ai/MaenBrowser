# Security audit — review batch, 2026-11-05

Scope: [pinned baseline and evidence](MAENBROWSER_CURRENT_STATE_AUDIT.md). Source audit and selected automated checks are not a penetration-test certification. No Sandbox, Site Isolation, TLS validation or Family Protection removal.

| Priority | Finding | Action / acceptance |
|---|---|---|
| P0 release gate | Pinned CEF154 trails official stable CEF156 and active Chrome cohorts; no audited backport list | [Engine upgrade](ENGINE_UPGRADE_PLAN.md). Do not mark secure from a version comparison alone |
| P0 guard implemented | Optional updater could be enabled despite missing authenticated feed / rollback policy | CMake helper rejects ON; native compile-time guard rejects bypass. OFF still builds. Full secure updater remains a blocker |
| P0 data preservation implemented | Main-document probe did not exclude every embedded frame; shadow DOM can hide drafts/calls | Preserve iframe/frame/object/embed, any detected shadow root, >1024 walked elements, errors or missing privileged inspection API. Unit evidence only; runtime compatibility may reduce discard frequency |
| P0 Android policy consistency implemented | Exported standard Activity could create another instance with independently cached Family settings | singleTask routes normal external intents through existing onNewIntent. External links open a new tab within the same Activity or visibly refuse at the existing tab budget; cold-start restore is preserved. New instrumentation verifies identity/intent delivery/new tab; compiled, not run locally. OS process-death/failure recovery remains acceptance work |
| P1 supply-chain gap | Binary SHA pins establish matching bytes, not trustworthy provenance | Rebuild codec CEF from pinned source, record toolchain/GN flags/SBOM, sign complete bundle; no third-party replacement DLL mixing |
| P1 automation surface | Opt-in CDP can operate authenticated browser | Separate profile, loopback, Family restrictions remain; no public bind, wildcard origins, or imported tokens. Vendor integration must explicitly support connection |
| P1 local privacy | Clock/weather uses OS location or timezone-derived coarse city plus MET Norway request | Existing user-requested default preserved; saved opt-out, cache/backoff. Document source, attribution and network disclosure; no analytics added |
| P1 extension privileges | Powerful extension APIs may expose all sites | Explicit user install/permissions, per-extension host review, inspect MV3 service worker and update provenance; no silent sideload from webpages |
| P1 download robustness | Authenticated/POST requests cannot safely be replayed in another engine | Preserve original request/browser and permission context. Baseline CI covers GET/POST/redirect; production network/account cases still required |

## Threat boundaries

Untrusted websites cannot call OS commands through a generic JS bridge. Existing native control endpoints require the specific internal extension origin, not Origin:null or a forged ordinary page. DPAPI/AndroidKeyStore protect local policy secrets against casual file reads; a compromised OS administrator or process can still defeat application boundaries. Resource modes must retain Chromium security defaults; user-supplied insecure launch arguments are outside safe supported operation.

No cross-engine cookie/token copying, certificate-error ignore handlers, authentication challenge bypasses, remote arbitrary scriptlet feeds, public automation server, hidden browsing telemetry or security setting changes were introduced. Native sandbox linkage and platform mitigations must be rechecked after every engine/toolchain upgrade. Family filtering is layered best-effort classification, not a guarantee that every harmful page is recognized.

## Authenticated update design (not implemented)

1. HTTPS-only feed on a publisher-controlled origin; bounded redirects to allowlisted HTTPS, size/time limits and no cross-origin credentials.
2. Canonical signed metadata: app/product ID, channel, platform/architecture, monotonically increasing release sequence and security epoch, app/engine version, package SHA256/size, issued/expiry UTC, minimum supported updater, transaction ID. Verify a pinned Ed25519 key before interpreting download URLs. Configure WinSparkle's EdDSA public-key API if retained; an appcast URL alone is insufficient.
3. Verify package bytes and Windows Authenticode chain/pinned publisher identity; Android uses OS signature lineage/Play delivery. App version, signed sequence and security floor must all pass; never install unsigned security fixes.
4. Store accepted sequence/floor in protected state (DPAPI/OS installer service ACLs); reject stale/replayed metadata and expired feeds. A compromised system clock requires a signed-sequence rule rather than trusting clock alone. Explicit key rotation signed by a pretrusted recovery key, no network TOFU.
5. Stage a complete matching engine bundle beside the active install. Quiesce processes, checkpoint profile-schema migration without logging secrets, atomically switch, health-check and commit. Retain only a previously signed build at or above the security floor; if none exists, fail safely and offer a signed repair package. Never roll back profiles to an incompatible schema silently.
6. Test interrupted download, proxy/TLS failures, invalid signature, wrong architecture/product, corrupt package, locked files, power loss before/after switch, expired metadata, rollback below floor, key rotation and failed startup. Canary channel before security-wide rollout; no arbitrary remote commands.

Trust roots, signing service, recovery ownership and installer transaction are publisher prerequisites. Until implemented and tested, updater enablement is intentionally blocked and manual publisher-signed releases are required. [WinSparkle EdDSA](https://github.com/vslavik/winsparkle/wiki/Upgrading-from-DSA-to-EdDSA-signatures), [Chromium extension security](https://chromium.googlesource.com/chromium/src/+/main/extensions/docs/security_faq.md), [WebView file inclusion risks](https://developer.android.com/privacy-and-security/risks/webview-unsafe-file-inclusion).

## 1.13.0 security delta

Added signed data transport, native atomic DPAPI bundle/native sequence/PIN validation, lazy Family classification loading, Store/update gates and current [CISA evidence](research/2026-11-06/browser-advisories.json).126 historical records are not126MaenBrowser vulnerabilities or fixed exploits; KEV is not an exhaustive CVE database. Vendor/product filtering excludes unrelated SD-WAN Edge records. Native signature trust boundary and OS/profile-rollback limits are documented in SIGNED_SECURITY_DATA.md. Engine binary upgrade and physical/native acceptance remain blocked, not declared solved.

Android1.13.0 also removes independent parental-state caches across Activity recreation: shared state plus one ordered writer and main-thread policy observers. SingleTask alone was insufficient for rotation/late-write overlap. This is source-reviewed and Android-compiled; real-device race/lifecycle acceptance remains required.
