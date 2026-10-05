# MaenBrowser Security Model

MaenBrowser uses defense in depth. No browser can guarantee that compromise is impossible.

## Release invariants
- Production Windows CI builds with the CEF sandbox enabled.
- TLS/certificate validation is not bypassed.
- Site isolation is not disabled by MaenBrowser resource modes.
- Security is never traded for lower RAM use.
- CEF/Chromium must be updated as security releases are adopted and tested.
- The browser has no mandatory Maen account or proprietary credential server.
- Search-choice and Quick Access destinations are not preloaded in the background.

## Windows hardening
Before CEF initialization, MaenBrowser requests native Windows DEP, ASLR relocation policy,
and legacy extension-point disabling. These are low-overhead OS mitigations; there is no
always-on Maen antivirus/background scanning service. Chromium's JIT is deliberately not
blocked because doing so would break modern websites.

CEF sandbox remains the primary web-content process isolation boundary. The build must fail
if the production workflow disables it.

## User data
The profile is local to the current Windows user. MaenBrowser does not add plaintext
credential logging or a Maen cloud credential store. Password/session handling supplied by
the Chromium runtime remains inside the pinned CEF/Chromium security model. A future
Maen-owned password manager must use reviewed OS-backed encryption (for example DPAPI) and
must receive a separate security review before release.

## Permissions, downloads, extensions
Website permissions and dangerous-download behavior must preserve Chromium security prompts;
MaenBrowser must not silently auto-grant camera, microphone, location, notification, or
similar sensitive permissions. Download completion must never auto-execute a downloaded file.
Extension management must not be reintroduced with silent installation or silent broad
permissions. CEF 152 legacy programmatic extension APIs remain disabled until a supported,
reviewed implementation exists.

## Private browsing
Private browsing reduces local persistence; it is not anonymity and cannot protect a user
from a compromised operating system, administrator-level malware, phishing, or credentials
voluntarily disclosed to a malicious site.

## Compatibility
MaenBrowser targets the widest practical range supported securely by its current Chromium/CEF
engine. It does not claim Windows 7/8/8.1 compatibility with Chromium 152. Shipping an obsolete
engine merely to support obsolete Windows would weaken security and is not acceptable.
Android is a separate future native target and is not produced by this Windows CEF build.
Its minSdk/targetSdk and supported Android range must be chosen against the current secure
mobile engine and Google Play requirements at implementation time.

## Reporting
Do not advertise MaenBrowser as impossible to hack. Security defects should be treated as
release-blocking when they affect sandboxing, TLS, credentials, permissions, updates, or
profile confidentiality.
