# MaenBrowser Android 1.11.0 preview

Actual native Android app, Android10/API29+, target/compile36. Small Java UI using
Android System WebView: keep the provider and Android security patches updated.
No Chromium binary is bundled. No desktop extension compatibility is claimed.

## Reproduce

Install JDK21 and Android SDK platform36. From this directory:

```
./gradlew :app:testDebugUnitTest :app:lintDebug :app:lintRelease :app:assembleDebug :app:assembleDebugAndroidTest :app:bundleRelease
```

On Windows use `gradlew.bat`. The checked-in Gradle wrapper pins both version and
SHA256. No machine paths/proxies/signing keys are committed. Filter assets are
copied by Gradle from the root pinned domain lists, not downloaded at runtime.

- `app/build/outputs/apk/debug/app-debug.apk`: debug-signed `.preview` app, for QA.
- `app/build/outputs/bundle/release/app-release.aab`: UNSIGNED unless the publisher
  supplies signing environment variables. Not directly publishable unsigned.
- `./gradlew :app:connectedDebugAndroidTest`: on a connected API29+ device; the
  checked-in instrumentation checks real startup/recreation and security settings.

## Publisher signing

Provisional release ID `ai.maenish.maenbrowser` must be confirmed before first
publication. Existing published apps require their original ID/signing lineage
and a versionCode higher than the store's current one (currently11100 here).
Store publisher credentials in your local secret store or GitHub Actions secrets,
never in source/chat. Environment variables used only by release signing:
`MAEN_ANDROID_KEYSTORE` (path), `MAEN_ANDROID_STORE_PASSWORD`,
`MAEN_ANDROID_KEY_ALIAS`, `MAEN_ANDROID_KEY_PASSWORD`. Build `:app:bundleRelease`
and verify with `jarsigner -verify`. Debug keystore is never a publisher key.

Play requires upload signing/Play App Signing, listing/privacy-policy URL,
contact, screenshots, content-rating/Data Safety declarations, required account
verification/testing and review. The public policy must reflect WebView/Safe
Browsing behavior. See `PRIVACY_TEMPLATE.md`; replace publisher placeholders and
host it before submitting. No Play upload/approval is implied by a successful build.

## Boundaries / device acceptance

HTTPS-only. No ignoring certificate failures, native JS bridge, unrestricted
file access or background crawler. Third-party cookies are opt-in outside family
mode; some login providers reject WebView regardless. File uploads use a user
picker; authenticated/blob downloads are limited. Downloads are never auto-opened.
Family filter is local domains, not on-device content classification or device
management. It cannot prevent app uninstall/data clearing, VPN use in another app,
or every harmful post on an otherwise allowed shared domain. Protect the device
with Android parental controls too. No claim of invisible advertising blocking.

Test low-memory physical devices and current devices for MP4/H264/AAC, WebM/VP9,
AV1/HEVC where supported, audio, full-screen video, calling/permissions, login,
uploads, battery, touch/keyboard/RTL and recovery. Remote desktop CDP integration
is Windows-only; this mobile edition does not open a debugging port in release.

For a manual publisher-signed CI artifact, configure `ANDROID_UPLOAD_KEYSTORE_BASE64`,
`ANDROID_STORE_PASSWORD`, `ANDROID_KEY_ALIAS`, `ANDROID_KEY_PASSWORD` in your
repository's Actions secrets, then run **Build publisher-signed Android bundle**.
It writes the key only to runner temp, deletes it after the build, verifies the
AAB signature and uploads a signed artifact. It does not submit to Play, and the
publisher must confirm applicationId/versionCode/listing before submission.
