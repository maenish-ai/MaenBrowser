# Security

- Modern Chromium/CEF security boundaries are kept enabled.
- MaenBrowser does not use `--no-sandbox` when CEF bootstrap sandbox support is
  available.
- Site isolation is not deliberately disabled to reduce RAM.
- Arbitrary CRX/ZIP extraction is not accepted without package verification.
- Automatic updates remain disabled until a signed HTTPS update feed exists.
- Browser profile data is local to the Windows user under `%LOCALAPPDATA%`.
