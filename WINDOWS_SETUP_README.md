# Windows Setup via GitHub Actions

Push this repository to `main`, then open **Actions → Build MaenBrowser Windows Setup → latest run → Artifacts**. Download `MaenBrowser-1.9.5-Windows-x64-Setup` and extract its `MaenBrowser-1.9.5-Setup.exe`.

Without CEF secrets, the workflow downloads official stock CEF and builds an installable browser. **Stock CEF does not guarantee H.264/AAC playback inside the CEF tab.** This is not the media-certified release. To enable media-certified builds, configure both `MAEN_CEF_ARCHIVE_URL` and `MAEN_CEF_ARCHIVE_SHA256` for the verified custom CEF 152 runtime; the workflow then performs strict H.264 playback testing.

A successful Windows-hosted Actions run is necessary to confirm the installer. The ZIP is source code, not an already compiled Setup.
