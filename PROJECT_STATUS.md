# Current status

Version identity: **MaenBrowser 1.0.0**

Implemented in source foundation:
- Windows C++/CEF bootstrapping
- Persistent local profile
- Extension loader base for unpacked web extensions
- Updater integration seam
- Windows installer/uninstaller
- Source sanity CI

Not yet complete enough to call a public binary release:
- custom MaenBrowser UI
- tested tab memory suspension/discard
- password manager UI/storage
- CRX3 verification
- signed release/update infrastructure
- Android application

The repository intentionally distinguishes implemented code from planned features so future releases are auditable.
