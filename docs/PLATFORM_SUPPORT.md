# Platform support

## Windows desktop — current release line

MaenBrowser 1.5.1 is a native Windows x64 desktop application using CEF/Chromium 152.
The GitHub workflow produces:

- `MaenBrowser-1.5.1-Setup.exe` — the normal double-click installer.
- `MaenBrowser-1.5.1-Windows-x64-Portable.zip` — optional portable build.

The installer installs under the current user's Local AppData, creates Start Menu and
Desktop shortcuts, registers Installed Apps/uninstall information, registers MaenBrowser
as a browser candidate for HTTP/HTTPS, and adds an App Paths registration.

Windows decides default-app ownership. MaenBrowser does not silently take over HTTP/HTTPS.

### Supported Windows target

The maintained target is modern supported 64-bit Windows. The project does not claim
Windows 7/8/8.1 support with Chromium/CEF 152. Supporting obsolete Windows safely would
require a separate legacy engine branch with its own security implications and is not
shipped as part of this release.

## Android

Android is a separate application target, not the Windows CEF binary repackaged as an APK.
The current repository does not yet contain a production Android browser and therefore
does not claim old/new Android compatibility.

When Android development begins it should use an Android-supported web engine and Play
policy-compliant packaging, with explicit minimum/target SDK levels and device testing.
No external Windows updater or executable-code download mechanism should be reused on Android.
