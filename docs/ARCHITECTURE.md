# Architecture

## Browser engine
CEF 152 / Chromium 152, Chrome runtime and Chrome-style UI.

## UI
CEF Chrome style provides the familiar Chromium tab strip, omnibox/address bar,
navigation controls and browser surfaces. This avoids duplicating a heavy UI
framework such as Electron.

## Storage
The Chromium profile is rooted in `%LOCALAPPDATA%\MaenBrowser\User Data`.

## Extensions
Unpacked extensions are loaded through `CefRequestContext::LoadExtension`.

## Packaging
GitHub Actions -> CEF download/verification -> CMake/Visual Studio build ->
runtime staging -> NSIS installer -> GitHub artifact.
