# MaenBrowser 1.0.0 — Windows production foundation

MaenBrowser is a local-first Chromium/CEF browser project focused on low RAM pressure, broad modern-web compatibility and user-controlled extensions.

## What this package is
This is the first source foundation, not a falsely labeled finished public binary. It establishes the production architecture and the files required to build the Windows engine shell. Public 1.0.0 release gates are tracked in `docs/ROADMAP.md`.

## Current foundation features
- Native C++ Windows target (no Electron).
- Chromium via CEF Chrome runtime.
- Persistent local profile under `%LOCALAPPDATA%\MaenBrowser\User Data`.
- No MaenBrowser cloud account.
- Unpacked extension loader foundation (`manifest.json` directory).
- Optional WinSparkle integration point for signed updates.
- NSIS installer + complete uninstall path, including local MaenBrowser profile removal.
- Security baseline for sandbox, extension permissions and password storage.

## Build prerequisites
- Windows 10/11 development machine.
- Visual Studio 2022 with Desktop development with C++.
- CMake 3.21+.
- Current CEF Standard Distribution for Windows x64.
- NSIS for packaging.
- Optional WinSparkle SDK for updater integration.

## Configure and build
```powershell
.\tools\configure-win-x64.ps1 -CefRoot "C:\SDK\cef_binary_xxx_windows64"
.\tools\build-release.ps1
```

CEF requires its DLLs and resource files next to the executable. The build configuration uses CEF's CMake helper lists to stage them.

## Package
Place the complete Release runtime in `dist\`, then:
```powershell
.\tools\package-nsis.ps1
```

The NSIS installer registers MaenBrowser under the current user's Windows uninstall registry and produces a normal uninstall entry.

## Extensions
The foundation can load an unpacked web extension directory containing `manifest.json` using CEF `LoadExtension`. External `.crx` installation remains disabled until CRX3 signature verification and safe extraction are implemented; this is intentional security behavior, not a missing checkbox.

## Updates
Updater code is disabled by default until the project has a real HTTPS appcast URL and an EdDSA signing key. Enable only after configuring those release assets.

## Important production rule
Do not ship this foundation build as a public browser yet. It currently uses the no-sandbox development path for straightforward bring-up. Before public distribution, migrate the Windows target to CEF's current bootstrap/sandbox architecture and make that a release gate.

## GitHub Actions
The repository includes `.github/workflows/maenbrowser-ci.yml`. On the first push to GitHub it starts automatically, validates the repository/version, and publishes a downloadable source artifact. It can also be started manually from **Actions > MaenBrowser CI > Run workflow**.

> A distributable Windows `.exe` is intentionally not produced by CI yet because the Chromium/CEF SDK binaries are not vendored in this source foundation. The Windows release workflow will be enabled once the pinned CEF SDK/build dependency is added, so CI never pretends to have built a browser executable when it has not.
