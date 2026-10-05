# MaenBrowser 1.x delivery roadmap

## Foundation now in this package
- Versioned project: 1.0.0
- C++/CEF Chromium shell
- Local-only profile directories
- Chrome runtime enabled
- External unpacked extension loader foundation
- Optional WinSparkle updater integration point
- NSIS installer/uninstaller registered in Windows Installed Apps
- Security and memory architecture decisions

## Next implementation gates before a public 1.0.0 release
- Custom MaenBrowser tab/address UI and visual identity
- Tab lifecycle / memory-saver implementation with measured 4 GB RAM test matrix
- Bookmarks, history and downloads UI
- Local password manager backed by Windows DPAPI/Credential Manager
- Extension permissions UI and allow/deny controls
- CRX3 verification/extraction for external extension packages
- Session restore and crash recovery
- Stable signed updater feed and code signing
- CEF sandbox enabled in Release CI
- Automated functional/security tests
- Privacy page and third-party notices

## Android
Android is a separate target sharing product identity and policy, not the Windows binary. Google Play release/update constraints are handled in that project rather than by shipping Windows-style runtime code.
