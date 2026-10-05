# Project status — MaenBrowser 1.1.0

This source tree is the expanded Windows desktop development release. It is designed to compile against the pinned CEF/Chromium distribution in GitHub Actions and package an installable Windows build.

Major additions over the foundation: Chrome Runtime integration, persistent local profile/preferences, downloads flow, unpacked extension loading, Windows installer/uninstaller, automated CEF build pipeline, and automatic low-memory Lite Mode for <=6 GB systems.

Security boundary: Lite Mode does not turn off sandboxing or site isolation.

Before public production release: run the acceptance/RAM matrix, verify the pinned CEF build and extension APIs, code-sign binaries/installer, configure signed update infrastructure, and perform security/privacy review.
