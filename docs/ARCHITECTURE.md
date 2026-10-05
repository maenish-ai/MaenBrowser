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


## Unified browser architecture gate (1.3)

CEF 152 exposes both Chrome and Alloy runtime styles. Chrome style supplies mature browser
functionality; Alloy supplies less default browser UI and is the intended route for a
Maen-owned shell. MaenBrowser keeps Chrome style in 1.3 because removing it before equivalent
navigation/tab/download/history surfaces exist would regress the working browser.

The custom shell must be introduced behind a build/runtime gate and pass the acceptance matrix
before becoming default. This prevents a visual rewrite from breaking the known-good browser.

Feature order for that shell:
1. window + tab strip + omnibox/navigation;
2. lifecycle/discard controller;
3. bookmarks/history/download surfaces;
4. groups/workspaces/sessions;
5. optional vertical tabs/split view/reader/command palette;
6. extension-management replacement compatible with pinned CEF.
