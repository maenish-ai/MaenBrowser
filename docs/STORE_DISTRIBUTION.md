# Store and executable-update preparation

Version1.13.0, source candidate. No Windows Store/Play upload or GitHub write was performed. A store account, reserved identity, signing ownership, policy declarations and actual device tests cannot be fabricated in this environment.

## Windows

`installer/msix/AppxManifest.xml.in` declares x64 Windows.Desktop minimum10.0.19041, explicit browser branding, internetClient and restricted runFullTrust. HTTP/HTTPS protocol associations pass one quoted URL to the existing native URL/Family guard, without forcing Windows default-browser choice. File associations are not falsely registered where the CLI does not handle them. Full-trust capability needs Store review; supported WebView/CEF native behavior and profile migration must be tested inside the package. No claimed support for unsafe/out-of-support OS versions simply because hardware is old.

Configure publisher-owned fields in `config/store-identity.json`, privacy/support HTTPS URLs and confirmed identities. Build the matching runtime/application with sandbox, resource stamping and acceptance checks. Then:

```powershell
pwsh tools/package-msix.ps1 -PayloadDirectory dist -OutputDirectory store-output
```

This uses actual Windows SDK MakeAppx, creates correctly sized PNG assets from the project icon, validates the manifest and produces explicitly named **UNSIGNED** review output plus SHA256. It cannot install itself as a trusted security update. This command has not been executed here because Windows SDK and a new native build are absent.

For publisher-signed direct distribution, use an installed CurrentUser certificate with a private key whose Subject matches the configured publisher; the script accepts its public thumbprint, not a password on a command line:

```powershell
python tools/audit-engine.py --output engine-current.json
pwsh tools/package-msix.ps1 -PayloadDirectory dist -OutputDirectory store-output -CertificateThumbprint PUBLISHER_CERTIFICATE_THUMBPRINT -EngineReport engine-current.json
```

Signing requires fresh matching engine evidence and completed production acceptance. SignTool validates the package; configured HTTPS packageBaseUrl permits `.appinstaller` generation with on-launch prompt/background update checks. Windows verifies the signed package/publisher/version. No `ForceUpdateFromAnyVersion` downgrade is enabled. Recovery uses an accepted signed version/security floor or a newer repair release; OS Store rollback behavior is not a custom WinSparkle transaction implementation.

Microsoft Store delivery uses Microsoft's signing/update mechanism and user update preferences. App Installer is an optional direct-distribution route. Legacy WinSparkle remains off and compile-blocked until a separately verified executable updater exists. No unsigned background executable install or self-modifying script updater.

## Android / Google Play

Current namespace remains `ai.maenish.maenbrowser`; provisional application ID is unchanged until publisher confirmation. Gradle reads the confirmed application ID from store-identity.json, derives versionCode11300 from1.13.0 and keeps debug `.preview` separate. Confirm an ID **before** production publication; changing it later changes application identity and may strand existing data. Do not silently migrate by copying cookies or credentials.

Local outputs: debug APK and unsigned release AAB. Debug uses a local test certificate; it is not a publisher production signature. Existing manual signed workflow now checks Android-specific identity/privacy prerequisites before using publisher upload-key secrets. It builds/tests/lints, verifies AAB signing and hashes artifacts; it does not submit to Play. Play App Signing ownership and final package acceptance remain publisher tasks. Do not upload the unsigned AAB as a completed production submission.

Required: correct target SDK policy at submission, privacy policy/Data Safety, camera/microphone/file-download explanations, screenshots, content/Family policy declarations as applicable, accessibility/RTL and actual device/WebView/media/permission tests. No Chrome-extension claim for Android WebView. Native app updates/filters are delivered by signed app/Play updates, not the Windows companion.

## Production gate

`tools/store-readiness.py --platform android|windows|all` lists actual missing metadata. `release-readiness.py` requires matching fresh engine evidence, publisher/advisory/physical acceptance and confirmed identities for a unified production release. Tag builds are gated; normal source/build testing remains available. Booleans must represent actual external evidence, never be set merely to silence a build. Keep parent/user profile data through upgrades and test legacy installer→MSIX migration explicitly.

References: [Microsoft MSIX](https://learn.microsoft.com/en-us/windows/msix/overview), [App Installer update settings](https://learn.microsoft.com/en-us/uwp/schemas/appinstallerschema/element-update-settings), [Google Play target SDK](https://developer.android.com/google/play/requirements/target-sdk), [Play App Signing](https://support.google.com/googleplay/android-developer/answer/9842756).

Protocol schema reference: [Microsoft uap3 Protocol](https://learn.microsoft.com/en-us/uwp/schemas/appxpackage/uapmanifestschema/element-uap3-protocol). Installed protocol/default-browser activation remains a Windows acceptance case.
