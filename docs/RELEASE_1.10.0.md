# MaenBrowser 1.10.0 — filtering, media coverage and opt-in agent integration

Built on the user's successful 1.9.10 source, commit
8fc1aa71df0876cf86e75b976dc92da65db73787. The working CEF 154 H.264/AAC archive,
its SHA-256 pin, sandbox and GPU safety policy are unchanged.

## Advertising and family data

Bundled ad coverage grows from 72,524 to 145,368 unique domains by merging a pinned
HaGeZi Light snapshot with the existing reviewed list. The merge is reapplied after
manual ad-list updates and at startup, so an update cannot discard the extra list.
Request-time lookup remains the sorted native DomainRules table; there are no
per-request regular-expression scans, downloads or content-script observers.
Site ad exceptions remain usable and do not override Family Protection.

Two small pinned uBlock Origin compatibility resources replace already-blocked
Google Analytics analytics.js and Google Publisher Tag gpt.js on exact HTTPS hosts
and paths. This keeps common library APIs/callbacks available without ad delivery,
tracking requests or impression pings. A family block is never replaced by a shim,
and ads-off/site-exception requests are not shimmed. No generalized scriptlet
engine, bait hiding or guarantee of invisible ad blocking is provided. Sites may
still detect missing ad requests or empty slots; first-party/video ads are not
universally removed. GPL notices and original source links accompany these separate
resources and the HaGeZi dataset.

The pinned StevenBlack adult snapshot was refreshed (76,796 domains); the existing
UT1 violence data and explicit parent allow/block lists remain. Category datasets
load only when Family Protection is enabled. These are domain categories, not an
AI classifier of every page/image/post on shared domains. Parent-approved-sites-only
mode remains the strongest mode. Snapshot refresh is an explicit maintenance tool,
not a background cloud service.

## Media and creative sites

The same proven codec-enabled engine now has required playback coverage for H.264
Main/High and Opus/Ogg, alongside existing H.264 Baseline/AAC, VP8/Vorbis, VP9/Opus,
AV1/Opus, MP3, FLAC, WAV and Ogg Vorbis. Windows tests also decode PNG, JPEG, WebP,
GIF, BMP, SVG and AVIF images. ALAC, AIFF, WMA, Matroska/H.264 and MPEG-2 are optional
probes recorded honestly in media-formats.json, not assumed to work or used to
silently substitute another application. HEVC/DRM and other platform codecs remain
conditional. Adding a file extension is not adding a decoder.

Media diagnostics lists more MIME/codec declarations and provides a user-triggered
1080p MediaCapabilities estimate for support/smoothness/power efficiency. These are
engine estimates, not decoded-file or hardware-performance proof. GPU/WebGL/WebRTC/
WebCodecs safety settings are preserved. No background transcoder or second codec
engine was added. Actual Adobe application workflows remain a device acceptance task.

## Navigation and branding

User-clicked ordinary cross-origin popup links now navigate in the existing tab.
A second OS URL launch into an existing profile reuses the focused browser tab
instead of CEF's default additional Chrome-style window. A successful singleton
handoff no longer displays an initialization error. Known OAuth/payment/login hosts
and paths preserve popup/opener behavior, including
loopback callbacks. Parent navigation rules are checked first. Native and extension
popup policies both preserve authentication targets. Explicit new tabs/windows and
unrecognized script-driven flows are not universally forced into one tab.

Settings and About menu entry points lead to MaenBrowser pages; advanced native
privacy/site-permission/extension controls remain accessible and family restrictions
remain enforced. Stale hard-coded 1.9.6 native API versions now use the authoritative
kVersion. Windows titles/icons already use MaenBrowser. CEF/Chromium internal pages,
third-party attribution and web compatibility identifiers retain engine branding;
complete rebranding of the compiled Chromium UI needs a separately built runtime.

## YourAssistantAgent integration

A separate companion patch is supplied for the actual maenish-ai/YourAssistantAgent
repository. It recognizes MaenBrowser.exe, adds it to browser selection/window
control, and launches selected MaenBrowser via a local CDP adapter rather than
Chrome's unsupported pipe launch. Explicit MaenBrowser selection never silently
falls back to Chrome. Login, 2FA and checkpoints remain owner actions; posting
approval rules in the Agent are untouched.

The adapter checks the installed capability marker before starting any debugger,
refusing old browsers that cannot isolate this profile. The browser's explicit --maen-agent-mode accepts a separate absolute user-data-dir
and prevents overlap with the normal browser profile. Shared native parent/ad
settings remain outside that isolated cookie profile. The mode refuses startup
when Family Protection is on or its settings changed during startup; native
settings writes from agent-controlled pages are denied. A single agent-only UI
watcher checks protected-file metadata once per second and closes owned agent
windows if settings are changed/deleted or become unreadable. It captures the
baseline before policy initialization to avoid a stale-startup race. Normal
browsing has no new poll or default debug port. The debug interface is loopback,
remote-origin wildcard flags are removed, and only a user-launched adapter owns
and closes its child process. CDP gives trusted local software control of the
isolated account profile; it is not an Internet-accessible service.

The Agent patch must be built/installed separately; it cannot change an old
installed Agent executable. Its optional extension bridge is not auto-installed.
Firefox extensions and every Chrome/Edge extension API/store are not made binary
compatible by this release. Chromium MV3 extensions still need individual testing.

## Validation limits

Portable source/native-policy tests, pinned-data hashes, shim callback tests,
PowerShell checks and mocked Agent tests are run in Linux. Windows compile/link,
new runtime/codec/shim integration tests, actual Facebook login, Adobe workflows,
all extension stores and user-device resource measurements remain pending.
CI keeps all security and real playback gates. This source release is not an
assertion of universal compatibility or an already-built Windows Setup.
