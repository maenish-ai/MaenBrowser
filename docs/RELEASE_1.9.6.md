# 1.9.6 source release candidate

Base: `9e407cf3251ac6ffb08923d5808087384dae850d` (main, 1.9.5).
No remote repository or user profile was modified during preparation.

## Protection
The homepage no longer has ad controls or a protection API request. The extension
retains its identity and settings. Native `save` requires `expectedRevision` from
the last displayed state; old panels/workers cannot silently write. Failed or timed
out writes are never replayed. The UI sends only changed fields, refreshes clean
forms on focus/settings events and tells dirty forms to reload after a conflict.
Badge updates do not require an always-awake worker. Popup distinguishes site
exceptions and filter failures. Revision conflict is E113; filter/recovery/partial
application failures have specific bilingual codes.

`controls.dat` stays DPAPI-protected (schema 1, backwards-compatible reading).
Atomic replacement retains the previous version at `controls.dat.bak`. Corrupt or
missing current settings with an existing backup stay locked. Explicit recovery
requires the backup PIN and always restores with family mode enabled. Backups
without a PIN are deliberately not used to unlock protection. This is a previous
version backup, so its most recent change may differ from the current file.
A bounded in-memory journal records only ad/family state, revision and uptime,
never URLs, domains, request content or PINs. It is included in on-demand technical reports.

Family rules use an immutable shared snapshot created before enabling family
mode and released when no requests reference it. Missing category lists still
prevent category-mode activation. Explicit allow-only mode remains available.

Manual ad refresh uses the existing StevenBlack hosts source via HTTPS, no cookies,
no redirects, a 30-second timeout and an 8 MiB download cap. Both JS and C++ enforce
count/size/domain constraints; native validates strict sorted uniqueness before
atomic replacement. Broken downloads keep active rules. Startup falls back to the
last valid cached or bundled list. This is HTTPS source/format validation, not a
publisher digital signature. Family lists are updated with reviewed app releases.

## Resources
The automatic selector picks UltraLite for <=2304 MiB physical memory, Lite for
<=6144 MiB or <=4 logical processors, Balanced otherwise, and Performance at >=16 GiB
and >=8 processors. A validated non-security mode hint applies the manual override
at restart. Protected settings contain the preference as well.
UltraLite cache ceilings: 32 MiB disk / 16 MiB media; Lite policies remain 64/32 or
96/48 MiB; Balanced 256/128 MiB. These are cache policies, not RAM ceilings.

Idle checks are opt-out and occur every two minutes, not continuous polling. Normal
idle duration keeps the existing setting (default 30 minutes). With memory load
>=85% or free RAM <384 MiB, safe candidates may be released after 5 minutes in
UltraLite or 10 minutes otherwise. At most two candidate inspections run per scan
under pressure, one otherwise. Downloads, active/pinned/audible/private/captured
or non-discardable tabs, selected conferencing hosts, forms and media retain
conservative exclusions and activity/configuration are rechecked before discard.
DOM inspection cannot prove that every web app has saved every private JS state;
keep-awake exclusions remain available and failures never trigger a discard.

Explicit saved-session restore opens only the first real site. Remaining tabs use
small local placeholders which navigate when selected. This affects Maen's saved
sessions, not Chromium's separate recently-closed-tab/session recovery.
The per-request context-handler allocation was removed, icons use shared resources.
No unmeasured URL-parser rewrite or forced renderer process limit was introduced.

## Security and compatibility
Build and startup require the CEF sandbox. Explicit known unsafe startup switches
are rejected. No GPU/video-decode/WebGL/TLS/site-isolation bypass is added. Existing
Windows DEP/ASLR controls and browser permission prompts remain in effect.
All declared extension permissions are currently used; none were removed merely
to reduce the permission count. No third-party antivirus is bundled.

Explicit main-frame user actions may request mailto/tel/zoommtg/msteams after a
native confirmation defaulting to No. Family mode and non-user navigation do not
launch them; all other native protocol executions remain denied. Which program
opens is controlled by Windows. No download auto-execution was added.

Media uses CEF in the same tab. WebView2's manual compatibility window remains
optional and requires an installed Runtime; no compulsory bootstrapper in Setup.
The release workflow requires the verified media-enabled CEF archive and actual
playback tests; no stock-codec bypass. Wider site, permission, hardware, HEVC and
DRM acceptance are explicitly recorded as pending in the checklist.

## Packaging
Version is 1.9.6; the worker entry is background-1.9.6.js. Setup staging excludes
test executables, debug/linker artifacts, old worker entry points and raw filter
source dumps while retaining licenses. Profile survives upgrades and uninstall.
Use README.md and WINDOWS_SETUP_README.md as current instructions; docs with older
version numbers are historical. Engine/runtime version is unchanged from the
reviewed base; this work does not certify that no newer security release exists.

Windows integration uses an isolated temporary profile and terminates only its own
process tree. It does not use or delete the normal browsing profile. It also verifies
that the encrypted ad-state preference survives an actual browser restart.
