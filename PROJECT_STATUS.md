# MaenBrowser 1.6.5 status

- Fixes the GitHub preflight false-positive caused by a doubled backslash literal for `cef_version.h`.
- Includes the Windows Media Foundation capability probe source and required Windows MF link libraries.
- CI always proceeds to a real Windows compile/Setup build: it uses verified custom media CEF when configured, otherwise pinned stock CEF for experimental compile/runtime testing.
- Stock CEF fallback is explicitly not claimed to provide H.264/AAC HTML5 playback; WhatsApp video remains a runtime test until a media-enabled CEF is supplied.
- Production sandbox, security hardening, adaptive resource policy, icon stamping, clean upgrades, and Setup-only distribution remain enabled.
