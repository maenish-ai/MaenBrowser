# MaenBrowser 1.11.0 — release candidate

This is a source and Android preview release. It is NOT a claim that the Windows
engine has current Chromium security fixes or that Android is approved for Play.

## Delivered

- Explicit desktop Performance setting alongside automatic/ultra/lite/balanced.
  Automatic conservative hardware selection and the existing form/chat/call idle
  safeguards remain enabled; no benchmark claim without device measurements.
- Saved desktop sessions preserve group titles/colors, load one page immediately,
  and defer remaining pages. JSON import/export is bounded, excludes private tabs
  and credential-bearing URLs, never exports cookies/passwords, and keeps one
  previous set for recovery. Link query tokens may be sensitive: protect exports.
- Named local CDP agent profiles via `tools/start-agent-browser.ps1 -SessionName`.
  Existing explicit loopback/debug/profile/family guards remain. This supports
  agents with an external CDP adapter; it cannot redirect a vendor's cloud browser.
- Windows subsequent DLL searches exclude the working directory; OS image policy
  rejects remote images and prefers System32. App-local CEF still loads, sandbox
  and JavaScript JIT remain available. This does not protect DLLs loaded earlier
  by the bootstrap or fix engine CVEs.
- Actual native Android application, API29 minimum, compile/target36, Gradle8.13,
  AGP8.13.2, native Java UI, system-updated WebView, no bundled Chromium/analytics/
  advertising SDK/background service. APK debug ID ends `.preview`; release ID
  `ai.maenish.maenbrowser` is provisional pending publisher confirmation.
- Android: adaptive tab budget (2/4/6), only selected restored tab initialized,
  explicit tab closing rather than discarding working chat/forms, durable cookies,
  encrypted address sessions and settings, authenticated encrypted parent policy,
  salted PBKDF2 passphrase with retry delays, fail-closed damaged policy, local ad
  and lazy family domain lists, HTTPS-only navigation, TLS errors rejected, Safe
  Browsing when system provider supports it, no file/native-JS bridge, explicit
  exact-origin camera/microphone grants, Android document picker and confirmed
  Downloads (no cookie forwarding). System media decoders are used.
- CI builds Windows Setup and Android APK/unsigned AAB; Android emulator startup
  and WebView setting checks target API29/36. Tagged releases are drafts AND
  prereleases; Android unsigned bundles cannot be uploaded to Play as-is.
- Official-engine freshness report plus scheduled artifact report; offline tests
  distinguish current versions from old builds. No automatic unverified DLL swap.

## Retained from 1.10.0

Codec-enabled CEF154 with verified H.264/AAC tests, image/audio fixtures,
145,368 local advertising domains, pinned compatibility shims, family lists,
YourAssistantAgent adapter contract, isolated profiles and normal-mode debugging
OFF. These are existing features, not new codec binaries in 1.11.0.

## Release blockers / acceptance still required

1. Windows is still pinned to Chromium154.0.8037.98. The official CEF index has a
   newer stable156 build at this check. Compile a matching newer codec-enabled
   engine and rerun Windows source/runtime/media/sandbox tests before presenting
   this as a current-security production browser. Stock CEF binaries cannot be
   mixed with old codecs. See `release-1.11.0/engine-audit.json`.
2. This Linux workspace cannot compile/run the Windows application. GitHub Windows
   CI must pass after upload; installer availability depends on that result.
3. Android needs actual device acceptance for MP4/H.264/AAC, camera, microphone,
   WhatsApp Web, uploads, OAuth/payment popups, background messaging and battery.
   Emulator tests check startup/settings, not all websites or playback quality.
4. Play publication needs the permanent application ID, publisher upload key,
   public privacy policy/contact/screenshots, content rating and Data Safety
   answers, account-specific testing/review. Do not share secrets in chat.

No universal codecs, all extensions, native Adobe/Office compatibility,
undetectable blocking, malware-free browsing or zero memory use are promised.
Android WebView has no Chrome/Firefox desktop extension host and websites can
reject WebView (notably some OAuth providers). No Flash or retired unsafe plugins.
