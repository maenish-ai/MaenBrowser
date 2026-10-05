# Security baseline

1. CEF/Chromium sandbox must be enabled for public release builds. This foundation currently uses the development no-sandbox path and is not a public-release binary.
2. No updater may accept unsigned payloads.
3. Update feeds use HTTPS; update packages use WinSparkle EdDSA signing when that updater is enabled.
4. External extensions are permission-gated. Proxy/VPN, all-sites access, downloads, native messaging and clipboard permissions receive elevated warnings.
5. CRX sideloading is disabled in this foundation until CRX3 signature verification and safe extraction are implemented.
6. Local passwords must use an OS-protected secret store (Windows DPAPI/Credential Manager). Plaintext password storage is prohibited.
7. Lite Mode must not disable TLS validation, sandboxing, site isolation or other Chromium security controls.
