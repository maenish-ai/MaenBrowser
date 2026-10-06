# Project status — MaenBrowser 1.5.19

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


## 1.5.19 Windows integration
- Added a normal double-click NSIS Setup executable as the primary Windows distribution.
- Added Installed Apps, Start Menu, Desktop, App Paths and Windows browser-capability registration.
- Added HTTP/HTTPS browser-candidate registration without hijacking the user's default-app choice.
- GitHub Actions publishes the NSIS Setup artifact only; the redundant Portable artifact is no longer produced.
- Preserved Lite Mode and CEF sandbox/security boundaries.
- Documented the honest platform boundary: current CEF 152 desktop line targets modern x64 Windows; Android requires a separate native implementation and is not falsely claimed complete.


## 1.5.19 smart download windows
- Tracks opener-created popup browsers without blanket-blocking popups.
- If a popup actually initiates a Chromium download, closes that temporary popup after the download event is created.
- Ordinary popups remain available for OAuth, sign-in and payment flows.
- Download ownership remains with Chromium's download manager; no polling/background service was added.


## 1.5.19 search choice start page
- New local MaenBrowser start page presents Google, Bing, DuckDuckGo and Brave Search side by side.
- Search choice is user-controlled and stored locally in the start-page origin.
- Quick Access links: YouTube, Gmail, WhatsApp and Wikipedia.
- No search engine or quick-access site is preloaded in the background.
- The UI uses MaenBrowser-owned text cards rather than copying third-party logos/assets.
- Third-party services remain external destinations; no affiliation is claimed.


## 1.5.19 unified Windows icon
- Fixes the sandbox/bootstrap icon boundary: CI stamps the MaenBrowser ICO into the final copied MaenBrowser.exe.
- Installer deploys the same ICO beside the executable.
- Desktop and Start Menu shortcuts explicitly use that ICO.
- Installed Apps, browser candidate, capabilities and URL protocol registrations use the same ICO.
- Explorer association refresh remains enabled after install/uninstall.

## 1.5.19 security hardening
- Added low-overhead native Windows DEP/ASLR/extension-point process mitigations before CEF initialization.
- Kept CEF sandbox as a production release invariant.
- Added fail-closed CI/preflight guards for certificate/web-security/site-isolation weakening.
- Documented credential/profile/private-mode trust boundaries and secure compatibility policy.
- No antivirus daemon, telemetry service, or background scanner was added; Lite/Balanced/Performance modes remain intact.
- Android remains a separate future target; this package does not falsely claim an APK or universal legacy-OS support.

## 1.5.19 download reliability hotfix
- Removed browser/popup closing from `OnBeforeDownload`.
- Chrome Runtime now retains full ownership of the initiating browser during download handoff.
- Fix targets immediate `Canceled` downloads seen with redirect/popup-based flows such as GitHub Actions artifacts.
- OAuth/payment/login popups remain permitted; no blanket popup blocking was added.
- Added a preflight regression gate that rejects future `CloseBrowser` calls inside `OnBeforeDownload`.
- Security hardening, sandbox invariant, Lite/Balanced/Performance modes, Search Choice, native download UI delegation and unified icon are retained.

## 1.5.19 CI/preflight correction
- Fixed the exact GitHub Actions failure from run 37336424335.
- Removed the obsolete preflight assertion that still required `popup_browser_ids_`, `browser->IsPopup()` and `CloseBrowser(false)` after 1.5.1 intentionally removed them.
- Replaced it with a coherent download reliability gate: Chrome Runtime delegation is required, `CloseBrowser` is forbidden inside `OnBeforeDownload`, and normal OAuth/payment/login popups must not be blanket-blocked.
- No runtime security, resource-mode, search-choice, installer or icon regression was introduced.

## 1.5.19 clean upgrade
- Setup refuses to overwrite a running MaenBrowser.
- Existing program/runtime directory is removed before staging the new release, preventing stale CEF DLL mixing.
- `%LOCALAPPDATA%\MaenBrowser` user profile is preserved during upgrade.
- Shell registration and shortcuts are rebuilt for the new version.
- Full user-requested uninstall still removes program and profile data.

## 1.5.19 unified Windows identity + background download handoff
- Running CEF/Chrome Runtime windows receive the canonical Maen icon via `WM_SETICON`.
- Process uses stable AppUserModelID `MaenBrowser.Desktop` for taskbar grouping/identity.
- Bootstrap entry point now also applies the existing Windows process hardening.
- Download popups are never hidden or closed in `OnBeforeDownload`.
- If a popup initiated a download, it is hidden only after Chromium reports actual transfer progress; it remains alive in the background.
- The transient popup is closed only after download completion (or after a terminal failure if it had already been hidden).
- Main GitHub/source page remains visible while Chromium's native download manager owns the transfer.

## 1.5.19 CI regression-gate correction
- Fixed the exact preflight failure from GitHub Actions run 37352814847.
- The older 1.5.2 guard incorrectly rejected `popup_browser_ids_` even though 1.5.4 deliberately reintroduced it only for passive popup/download lifecycle tracking.
- CI now rejects the actual dangerous behavior: `CloseBrowser` or `SW_HIDE` inside `OnBeforeDownload`.
- Popup tracking remains allowed so a confirmed in-progress download popup can be hidden safely and closed only at a terminal download state.

## 1.5.19 Windows compile fix
- Fixed the exact GitHub Actions compiler failure from run 37354094549:
  `SetCurrentProcessExplicitAppUserModelID`: identifier not found.
- Added the Windows Shell API declaration header (`shellapi.h`).
- Added explicit `shell32` linkage for the taskbar AppUserModelID call.
- Added CI preflight guards so this declaration/link dependency cannot be removed accidentally.
- Unified icon/taskbar identity, safe background-download lifecycle, clean upgrade and security/resource policies are retained.

## 1.5.19 AppUserModelID portability fix
- Fixed repeated C3861 failure from GitHub Actions run 37355525064.
- Replaced the direct SDK-gated `SetCurrentProcessExplicitAppUserModelID` call with runtime resolution from `shell32.dll` via `GetProcAddress`.
- This removes dependence on a particular Windows SDK declaration gate while retaining stable taskbar identity on supported Windows.
- If the API cannot be resolved, MaenBrowser continues safely with its stamped EXE icon and per-window `WM_SETICON`.
- Added a CI guard that forbids reintroducing the brittle direct call.

## 1.5.19 media compatibility diagnostics
- Added a local Media Diagnostics page covering H.264, AAC, VP8, VP9, AV1, Opus, Vorbis, MediaSource, EME, WebRTC, Picture-in-Picture, WebCodecs and WebGL.
- Added CI regression gates that forbid disabling GPU acceleration, accelerated video decode, WebRTC or MediaSource.
- Chromium/CEF media and GPU defaults remain enabled; no heavy external player, codec pack or background service was added.
- IMPORTANT: the official CEF binary distribution does not enable Chrome proprietary codecs by default. This release diagnoses that boundary; it does not falsely claim H.264/AAC support.
- Full H.264/AAC software support requires a custom CEF/Chromium build configured for proprietary codecs and a licensing review before redistribution.

## 1.5.19 media diagnostics compile correction
- Fixed GitHub Actions run 37360172454: undefined `PercentEncode` caused C3861 and cascading C2676.
- Uses pinned CEF's declared `CefURIEncode` from `cef_parser.h` instead.
- Added compile regression guard for this exact failure.


## 1.5.19 start-page visual refinement
- Added lightweight inline vector brand marks to all four Search Engine cards and all four Quick Access cards.
- Icons are embedded in the local start page: no image downloads, no background services, and no additional runtime dependency.
- Search behavior, local search-engine choice, Lite/Balanced/Performance resource policy, sandbox policy, and media diagnostics code remain unchanged.


## 1.5.19 adaptive performance/media pass
- Refined Lite Mode for <=4 GiB and 4–6 GiB systems with smaller cache ceilings.
- Added CPU topology to hardware classification without forcing risky GPU flags.
- Preserved Chromium GPU acceleration, WebGL, accelerated video decode and security isolation.
- Added a local start-page media/3D capability check for H.264, HEVC, VP9, AV1, AAC, Opus, FLAC, MP3 and WebGL2.
- Added `docs/PERFORMANCE.md` with measurable acceptance targets and honest codec/licensing boundaries.

## 1.5.19 WhatsApp/blob download continuation
- Explicitly continues accepted downloads through `CefBeforeDownloadCallback::Continue` instead of relying on implicit Chrome Runtime handling.
- Keeps the suggested filename/default download location and shows Save As to preserve user control.
- Targets WhatsApp Web/blob/service-worker media transfers that could immediately report Canceled in the embedded runtime.
- Does not close or hide the initiating popup in `OnBeforeDownload`; terminal-only cleanup remains unchanged.
- Does not weaken sandbox, TLS, site isolation, GPU/media acceleration, or the adaptive low-memory policy.
- Video playback codec support is a separate boundary; this change fixes download continuation, not proprietary H.264/AAC availability.

## 1.5.19 preflight correctness repair
- Fixed the exact 1.5.12 CI failure: the WhatsApp download regression gate naively rejected any `return false;` inside `OnBeforeDownload`, including the defensive null-callback guard.
- The gate now validates the real ownership path: `callback->Continue(CefString(), true);` followed by `return true;`.
- The defensive invalid-callback path no longer delegates a partially handled download; it returns without dereferencing an invalid callback.
- Runtime download, adaptive performance/media, sandbox, GPU/media acceleration and Windows integration behavior are otherwise preserved.


## 1.5.19 setup-only distribution
- GitHub Actions publishes only the Windows x64 NSIS Setup artifact.
- The separate Portable ZIP packaging/upload steps were removed to reduce CI time, storage, and release ambiguity.
- The installer still contains the complete CEF runtime required by MaenBrowser.


## 1.5.19 media release gate
- Treats WhatsApp/Web MP4 playback as a release blocker rather than a cosmetic issue.
- Adds a documented compatibility matrix for H.264/AAC, VP8/VP9, AV1, Opus/Vorbis, MP3/FLAC, MSE, WebRTC and WebCodecs.
- Preserves GPU/hardware decode and the adaptive low-memory policy; no external codec pack or background media service is added.
- Adds CI runtime-integrity checks and keeps Setup-only artifact publishing.
- Proprietary-codec redistribution remains gated on a version-matched custom CEF build plus licensing review; the public CEF binary is not falsely advertised as full H.264/AAC support.


## 1.5.19 CI + media-runtime correction
- Removes the hard-coded 1.5.14 GitHub Actions version check that caused run 37429335016 to stop before preflight/build.
- VERSION is now the single release-version source checked against CMake, the version header and NSIS, so the same drift cannot silently recur.
- Setup-only publishing remains intact.
- Preserves Chromium GPU acceleration, hardware video decode, MediaSource, WebRTC, WebCodecs, VP8/VP9, AV1 and standard audio paths.
- Adds an optional SHA-256-pinned custom CEF archive path (`MAEN_CEF_ARCHIVE_URL` + `MAEN_CEF_ARCHIVE_SHA256`) for a reviewed, version-matched media-enabled CEF runtime.
- The official CEF fallback remains available for normal CI; it is not falsely labeled as full H.264/AAC support.
- No random codec DLL, codec pack, TLS bypass, sandbox weakening or forced GPU override is introduced.


## 1.5.19 custom CEF media build path
- Added `tools/build-cef-media.ps1` with the Chromium/CEF media build flags `proprietary_codecs=true ffmpeg_branding=Chrome`.
- Added `docs/CUSTOM_CEF_MEDIA_BUILD.md` with version/provenance, WhatsApp runtime acceptance tests, and licensing boundary.
- Normal app CI remains lightweight; it consumes a reviewed custom CEF archive only when URL + SHA-256 are explicitly configured.
- This source change alone does not claim WhatsApp H.264 playback until the custom runtime is actually built, integrated and runtime-tested.
