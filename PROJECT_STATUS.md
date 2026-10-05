# Project status — MaenBrowser 1.5.3

This source tree is the expanded Windows desktop development release. It is designed to compile against the pinned CEF/Chromium distribution in GitHub Actions and package an installable Windows build.

Major additions over the foundation: CEF Chrome Runtime integration, persistent local profile/preferences, downloads flow, Windows installer/uninstaller, automated CEF build pipeline, automatic low-memory Lite Mode for <=6 GB systems, MaenBrowser Windows branding/icon resources, and a lightweight MaenBrowser start page. Programmatic unpacked-extension loading is intentionally disabled on the pinned CEF 152 API.

Security boundary: Lite Mode does not turn off sandboxing or site isolation.

Before public production release: run the acceptance/RAM matrix, verify the pinned CEF build and extension APIs, code-sign binaries/installer, configure signed update infrastructure, and perform security/privacy review.

## CI hardening - 2026-10-05
- Pinned GitHub Actions to `windows-2022` because `windows-latest` moved to a VS2026 image that does not provide the requested Visual Studio 17 2022 generator.
- Added an explicit VS2022 C++ toolchain check before CMake configuration.
- Explicitly builds CEF with `USE_SANDBOX=ON`.
- Added `tools/preflight.ps1` to catch repository/version/workflow drift before downloading/building dependencies.
- Chrome Runtime download handling now delegates to Chromium's native download UI; MaenBrowser still observes completion events.


## CI repair 2026-10-05
- Pinned runner: windows-2022 / Visual Studio 2022.
- Replaced Windows tar bzip2 extraction with deterministic two-stage 7-Zip extraction after the previous run stalled until the 90-minute job timeout.
- Exact CEF archive metadata match and SHA-1 verification retained.
- Repository preflight remains enabled before dependency download/build.


## CEF 152 compile repair
- Removed obsolete `CefSettings.chrome_runtime` and `persist_user_preferences` fields.
- Migrated browser preference writes to `CefPreferenceManager`.
- Removed calls to legacy `CefRequestContext` extension APIs absent from CEF 152.
- Added preflight regression guards for those removed APIs.
- Extension programmatic loading is intentionally disabled until implemented through a CEF 152-supported path; no false compatibility claim is made.

## Branding/UI pass
- Added MaenBrowser multi-size Windows icon resource and installer icon.
- Desktop/Start shortcuts explicitly use the application icon.
- Added a lightweight MaenBrowser start page instead of launching directly into Google.
- Window titles retain MaenBrowser identity.
- Kept the proven CEF Chrome Runtime frame for this build rather than risking a wholesale shell rewrite in one release.
- Corrected feature documentation so extension support is not overstated.


## 1.5.3 Windows integration
- Added a normal double-click NSIS Setup executable as the primary Windows distribution.
- Added Installed Apps, Start Menu, Desktop, App Paths and Windows browser-capability registration.
- Added HTTP/HTTPS browser-candidate registration without hijacking the user's default-app choice.
- Retained the portable ZIP as an optional secondary artifact.
- Preserved Lite Mode and CEF sandbox/security boundaries.
- Documented the honest platform boundary: current CEF 152 desktop line targets modern x64 Windows; Android requires a separate native implementation and is not falsely claimed complete.


## 1.5.3 smart download windows
- Tracks opener-created popup browsers without blanket-blocking popups.
- If a popup actually initiates a Chromium download, closes that temporary popup after the download event is created.
- Ordinary popups remain available for OAuth, sign-in and payment flows.
- Download ownership remains with Chromium's download manager; no polling/background service was added.


## 1.5.3 search choice start page
- New local MaenBrowser start page presents Google, Bing, DuckDuckGo and Brave Search side by side.
- Search choice is user-controlled and stored locally in the start-page origin.
- Quick Access links: YouTube, Gmail, WhatsApp and Wikipedia.
- No search engine or quick-access site is preloaded in the background.
- The UI uses MaenBrowser-owned text cards rather than copying third-party logos/assets.
- Third-party services remain external destinations; no affiliation is claimed.


## 1.5.3 unified Windows icon
- Fixes the sandbox/bootstrap icon boundary: CI stamps the MaenBrowser ICO into the final copied MaenBrowser.exe.
- Installer deploys the same ICO beside the executable.
- Desktop and Start Menu shortcuts explicitly use that ICO.
- Installed Apps, browser candidate, capabilities and URL protocol registrations use the same ICO.
- Explorer association refresh remains enabled after install/uninstall.

## 1.5.3 security hardening
- Added low-overhead native Windows DEP/ASLR/extension-point process mitigations before CEF initialization.
- Kept CEF sandbox as a production release invariant.
- Added fail-closed CI/preflight guards for certificate/web-security/site-isolation weakening.
- Documented credential/profile/private-mode trust boundaries and secure compatibility policy.
- No antivirus daemon, telemetry service, or background scanner was added; Lite/Balanced/Performance modes remain intact.
- Android remains a separate future target; this package does not falsely claim an APK or universal legacy-OS support.

## 1.5.3 download reliability hotfix
- Removed browser/popup closing from `OnBeforeDownload`.
- Chrome Runtime now retains full ownership of the initiating browser during download handoff.
- Fix targets immediate `Canceled` downloads seen with redirect/popup-based flows such as GitHub Actions artifacts.
- OAuth/payment/login popups remain permitted; no blanket popup blocking was added.
- Added a preflight regression gate that rejects future `CloseBrowser` calls inside `OnBeforeDownload`.
- Security hardening, sandbox invariant, Lite/Balanced/Performance modes, Search Choice, native download UI delegation and unified icon are retained.

## 1.5.3 CI/preflight correction
- Fixed the exact GitHub Actions failure from run 37336424335.
- Removed the obsolete preflight assertion that still required `popup_browser_ids_`, `browser->IsPopup()` and `CloseBrowser(false)` after 1.5.1 intentionally removed them.
- Replaced it with a coherent download reliability gate: Chrome Runtime delegation is required, `CloseBrowser` is forbidden inside `OnBeforeDownload`, and normal OAuth/payment/login popups must not be blanket-blocked.
- No runtime security, resource-mode, search-choice, installer or icon regression was introduced.

## 1.5.3 clean upgrade
- Setup refuses to overwrite a running MaenBrowser.
- Existing program/runtime directory is removed before staging the new release, preventing stale CEF DLL mixing.
- `%LOCALAPPDATA%\MaenBrowser` user profile is preserved during upgrade.
- Shell registration and shortcuts are rebuilt for the new version.
- Full user-requested uninstall still removes program and profile data.
