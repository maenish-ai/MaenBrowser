# MaenBrowser 1.9.9 — include the codec-enabled runtime in normal builds

The 1.9.8 Windows build at commit 24a5c31 produced
`MaenBrowser-Windows-x64-Setup-stock-CEF-preview`. Its success did not establish
H.264 support. With no custom-runtime secrets the build previously selected stock
CEF, explaining why upgrading source alone left MP4 playback unavailable.

Normal builds now download a specific Windows x64 codec-enabled CEF archive:
CEF 154.0.34+g14c5a08 / Chromium 154.0.8037.98 from psmux/godwinmix-cef. The URL,
SHA-256, size, platform, source commit and upstream build flags are recorded in
`config/media-runtime.lock.json`. It replaces the CEF 152 dependency as a complete
header/library/runtime set, rather than mixing unrelated DLLs. The upstream
maintainer declares H.264/AAC flags and unmodified CEF/Chromium sources; archive
identity verification is not an independent reproducible-build attestation.

No repository secrets or self-hosted Chromium builder are needed for this default
path. Both old custom-runtime secrets should be removed if they point to CEF 152;
optional overrides must supply both URL and SHA-256 and use the new exact version
and media marker. Partial or incompatible overrides fail explicitly.

The downloader checks HTTPS, SHA-256, size, safe extraction, distribution layout,
version and Windows x64 PE machine type. Extraction never reuses a stock runtime
by version alone. A provenance receipt records the archive identity and hashes of
critical extracted files; CMake verifies it again before compilation. This receipt
records download verification, not successful media playback. License and credits
files accompany Setup.

All Setup builds must run actual H.264/AAC and other codec playback tests. The
stock preview fallback and its skipped media tests have been removed. Tag v1.9.9
creates a draft GitHub release only after the Windows build/tests pass. H.264
hardware acceleration remains under Chromium's normal GPU policy; no extra engine
or persistent content script is added. Sandbox, site isolation, TLS, family rules,
ad filtering and profile protection remain enabled as previously implemented.

Validation: downloaded Windows archive SHA-256 matches the pinned release digest.
Portable tests and actual archive extraction/layout checks are recorded separately.
This Linux preparation environment cannot run the Windows browser, so it does not
claim that runtime playback, WhatsApp/Adobe compatibility or device performance
have passed. CI must confirm these before release. HEVC, DRM/Widevine and every
possible Internet codec are outside this H.264/AAC fix.
