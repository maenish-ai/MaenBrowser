# Feature gap matrix

Baseline1.12.2 at `e2692a8`, review branch as of2026-11-05. Evidence scopes: [audit](MAENBROWSER_CURRENT_STATE_AUDIT.md), [tests](TEST_RESULTS.md), [community CSV](COMMUNITY_REQUESTS.csv), [competitor sources](COMPETITOR_FEATURE_COMPARISON.md). The rows below cover all master-request families, including items with no sampled community evidence: their justification is the explicit product/engineering acceptance requirement, not invented demand. “Not verified” means no safe implementation/acceptance conclusion, not necessarily unavailable in Chromium. No untested feature is advertised as supported.

## A — Tabs and productivity

| Requirement | State / gap | Code evidence / engineering reason | Lane |
|---|---|---|---|
|Vertical Tabs|Not verified for exact behavior; needs supported API/native design and functional acceptance|`assets/companion/tabs.js; tab-policy.js; memory-policy.js; session-restore.js`; user acceptance requirement|P2|
|Horizontal Tabs|Present native Chrome Runtime; baseline browser CI, detailed UI acceptance pending|`assets/companion/tabs.js; tab-policy.js; memory-policy.js; session-restore.js`; user acceptance requirement|P2|
|Tab Groups|Partial: companion/API paths; full native interactions need acceptance|`assets/companion/tabs.js; tab-policy.js; memory-policy.js; session-restore.js`; user acceptance requirement|P2|
|Nested Tab Groups|Not verified for exact behavior; needs supported API/native design and functional acceptance|`assets/companion/tabs.js; tab-policy.js; memory-policy.js; session-restore.js`; user acceptance requirement|P2|
|Tab Stacks|Not verified for exact behavior; needs supported API/native design and functional acceptance|`assets/companion/tabs.js; tab-policy.js; memory-policy.js; session-restore.js`; user acceptance requirement|P2|
|Split View|Not verified for exact behavior; needs supported API/native design and functional acceptance|`assets/companion/tabs.js; tab-policy.js; memory-policy.js; session-restore.js`; user acceptance requirement|P2|
|Workspaces|Partial saved groups/sessions; no confirmed native isolated workspace architecture|`assets/companion/tabs.js; tab-policy.js; memory-policy.js; session-restore.js`; user acceptance requirement|P2|
|Workspace Rules|Not verified for exact behavior; needs supported API/native design and functional acceptance|`assets/companion/tabs.js; tab-policy.js; memory-policy.js; session-restore.js`; user acceptance requirement|P2|
|Automatic Tab Grouping|Not verified for exact behavior; needs supported API/native design and functional acceptance|`assets/companion/tabs.js; tab-policy.js; memory-policy.js; session-restore.js`; user acceptance requirement|P2|
|Tab Search|Partial: companion manager URL/title search, not full-text indexing|`assets/companion/tabs.js; tab-policy.js; memory-policy.js; session-restore.js`; user acceptance requirement|P2|
|Tab Preview|Not verified for exact behavior; needs supported API/native design and functional acceptance|`assets/companion/tabs.js; tab-policy.js; memory-policy.js; session-restore.js`; user acceptance requirement|P2|
|Tab Snoozing|Not verified for exact behavior; needs supported API/native design and functional acceptance|`assets/companion/tabs.js; tab-policy.js; memory-policy.js; session-restore.js`; user acceptance requirement|P2|
|Pinned Tabs|Present runtime APIs; persistence after upgrade not certified|`assets/companion/tabs.js; tab-policy.js; memory-policy.js; session-restore.js`; user acceptance requirement|P2|
|Sleeping Tabs|Partial: guarded idle discard, not every vendor sleeping-state implementation|`assets/companion/tabs.js; tab-policy.js; memory-policy.js; session-restore.js`; user acceptance requirement|P2|
|Tab Discard|Partial: guarded discard; uncertain pages preserved; new shadow/frame behavior unit-tested|`assets/companion/tabs.js; tab-policy.js; memory-policy.js; session-restore.js`; user acceptance requirement|P2|
|Tab Hibernation|Partial: guarded discard, manual requested refinements unverified|`assets/companion/tabs.js; tab-policy.js; memory-policy.js; session-restore.js`; user acceptance requirement|P2|
|Session Restore|Partial: URL/group/deferred snapshots; not arbitrary DOM/call state|`assets/companion/tabs.js; tab-policy.js; memory-policy.js; session-restore.js`; user acceptance requirement|P2|
|Saved Sessions|Present companion logic, portable tests; installed acceptance pending|`assets/companion/tabs.js; tab-policy.js; memory-policy.js; session-restore.js`; user acceptance requirement|P2|
|Auto Restore After Crash|Not verified for exact behavior; needs supported API/native design and functional acceptance|`assets/companion/tabs.js; tab-policy.js; memory-policy.js; session-restore.js`; user acceptance requirement|P0|
|Duplicate Tab Detection|Present companion logic; Windows runtime acceptance pending|`assets/companion/tabs.js; tab-policy.js; memory-policy.js; session-restore.js`; user acceptance requirement|P2|
|Close Duplicate Tabs|Present companion action; unsaved-state safety acceptance pending|`assets/companion/tabs.js; tab-policy.js; memory-policy.js; session-restore.js`; user acceptance requirement|P2|
|Quick Commands|Not verified for exact behavior; needs supported API/native design and functional acceptance|`assets/companion/tabs.js; tab-policy.js; memory-policy.js; session-restore.js`; user acceptance requirement|P2|
|Command Palette|Not verified for exact behavior; needs supported API/native design and functional acceptance|`assets/companion/tabs.js; tab-policy.js; memory-policy.js; session-restore.js`; user acceptance requirement|P2|
|Mouse Gestures|Not verified for exact behavior; needs supported API/native design and functional acceptance|`assets/companion/tabs.js; tab-policy.js; memory-policy.js; session-restore.js`; user acceptance requirement|P2|
|Keyboard Shortcut Customization|Not verified for exact behavior; needs supported API/native design and functional acceptance|`assets/companion/tabs.js; tab-policy.js; memory-policy.js; session-restore.js`; user acceptance requirement|P2|
|Reopen Closed Tabs|Not verified for exact behavior; needs supported API/native design and functional acceptance|`assets/companion/tabs.js; tab-policy.js; memory-policy.js; session-restore.js`; user acceptance requirement|P2|
|Tab History|Not verified for exact behavior; needs supported API/native design and functional acceptance|`assets/companion/tabs.js; tab-policy.js; memory-policy.js; session-restore.js`; user acceptance requirement|P2|
|Tab Sorting|Not verified for exact behavior; needs supported API/native design and functional acceptance|`assets/companion/tabs.js; tab-policy.js; memory-policy.js; session-restore.js`; user acceptance requirement|P2|
|Context-based Tab Management|Not verified for exact behavior; needs supported API/native design and functional acceptance|`assets/companion/tabs.js; tab-policy.js; memory-policy.js; session-restore.js`; user acceptance requirement|P2|
|Bookmark Folder to Tab Group|Not verified for exact behavior; needs supported API/native design and functional acceptance|`assets/companion/tabs.js; tab-policy.js; memory-policy.js; session-restore.js`; user acceptance requirement|P2|

## B — Accounts and isolation

| Requirement | State / gap | Code evidence / engineering reason | Lane |
|---|---|---|---|
|Multiple Browser Profiles|Partial separate launch profile directories; native UX testing pending|`src/storage/agent_profile.h; src/app/maen_app.cpp`; user acceptance requirement|P1|
|Work/Personal Profiles|Partial explicit separate directories, not automatic container rules|`src/storage/agent_profile.h; src/app/maen_app.cpp`; user acceptance requirement|P1|
|Website Containers|Absent identified request-context container routing; separate launch profiles not equivalent|`src/storage/agent_profile.h; src/app/maen_app.cpp`; user acceptance requirement|P1|
|Temporary Containers|Absent identified request-context container routing; separate launch profiles not equivalent|`src/storage/agent_profile.h; src/app/maen_app.cpp`; user acceptance requirement|P1|
|Cookie Isolation|Not verified for exact behavior; needs supported API/native design and functional acceptance|`src/storage/agent_profile.h; src/app/maen_app.cpp`; user acceptance requirement|P1|
|Site-Specific Profiles|Absent identified request-context container routing; separate launch profiles not equivalent|`src/storage/agent_profile.h; src/app/maen_app.cpp`; user acceptance requirement|P1|
|Profile Password Protection|Not verified for exact behavior; needs supported API/native design and functional acceptance|`src/storage/agent_profile.h; src/app/maen_app.cpp`; user acceptance requirement|P1|
|Session Isolation|Partial CEF profile/agent separation; WebView fallback separate; no token copying|`src/storage/agent_profile.h; src/app/maen_app.cpp`; user acceptance requirement|P1|
|Open Links in Selected Profile|Not verified for exact behavior; needs supported API/native design and functional acceptance|`src/storage/agent_profile.h; src/app/maen_app.cpp`; user acceptance requirement|P1|
|Automatic Container Rules|Absent identified request-context container routing; separate launch profiles not equivalent|`src/storage/agent_profile.h; src/app/maen_app.cpp`; user acceptance requirement|P1|

## C — Performance

| Requirement | State / gap | Code evidence / engineering reason | Lane |
|---|---|---|---|
|Automatic mode|Present mode selection/policy, portable tests; 4/8/16GB benefit not yet measured|`src/app/resource_mode.cpp; assets/companion/memory-policy.js; tools/measure-browser-processes.ps1`; user acceptance requirement|P1|
|UltraLite mode|Present mode selection/policy, portable tests; 4/8/16GB benefit not yet measured|`src/app/resource_mode.cpp; assets/companion/memory-policy.js; tools/measure-browser-processes.ps1`; user acceptance requirement|P1|
|Lite mode|Present mode selection/policy, portable tests; 4/8/16GB benefit not yet measured|`src/app/resource_mode.cpp; assets/companion/memory-policy.js; tools/measure-browser-processes.ps1`; user acceptance requirement|P1|
|Balanced mode|Present mode selection/policy, portable tests; 4/8/16GB benefit not yet measured|`src/app/resource_mode.cpp; assets/companion/memory-policy.js; tools/measure-browser-processes.ps1`; user acceptance requirement|P1|
|Performance mode|Present mode selection/policy, portable tests; 4/8/16GB benefit not yet measured|`src/app/resource_mode.cpp; assets/companion/memory-policy.js; tools/measure-browser-processes.ps1`; user acceptance requirement|P1|
|Whole-process RAM|Unmeasured on physical target tiers; benchmark protocol and collector supplied|`src/app/resource_mode.cpp; assets/companion/memory-policy.js; tools/measure-browser-processes.ps1`; user acceptance requirement|P1|
|Startup latency|Unmeasured on physical target tiers; benchmark protocol and collector supplied|`src/app/resource_mode.cpp; assets/companion/memory-policy.js; tools/measure-browser-processes.ps1`; user acceptance requirement|P1|
|Frozen tabs|Unmeasured on physical target tiers; benchmark protocol and collector supplied|`src/app/resource_mode.cpp; assets/companion/memory-policy.js; tools/measure-browser-processes.ps1`; user acceptance requirement|P1|
|Idle CPU|Unmeasured on physical target tiers; benchmark protocol and collector supplied|`src/app/resource_mode.cpp; assets/companion/memory-policy.js; tools/measure-browser-processes.ps1`; user acceptance requirement|P1|
|Heat|Unmeasured on physical target tiers; benchmark protocol and collector supplied|`src/app/resource_mode.cpp; assets/companion/memory-policy.js; tools/measure-browser-processes.ps1`; user acceptance requirement|P1|
|Battery life|Unmeasured on physical target tiers; benchmark protocol and collector supplied|`src/app/resource_mode.cpp; assets/companion/memory-policy.js; tools/measure-browser-processes.ps1`; user acceptance requirement|P1|
|Heavy sites|Unmeasured on physical target tiers; benchmark protocol and collector supplied|`src/app/resource_mode.cpp; assets/companion/memory-policy.js; tools/measure-browser-processes.ps1`; user acceptance requirement|P1|
|Chat responsiveness|Unmeasured on physical target tiers; benchmark protocol and collector supplied|`src/app/resource_mode.cpp; assets/companion/memory-policy.js; tools/measure-browser-processes.ps1`; user acceptance requirement|P1|
|Video performance|Unmeasured on physical target tiers; benchmark protocol and collector supplied|`src/app/resource_mode.cpp; assets/companion/memory-policy.js; tools/measure-browser-processes.ps1`; user acceptance requirement|P1|
|Tab switching|Unmeasured on physical target tiers; benchmark protocol and collector supplied|`src/app/resource_mode.cpp; assets/companion/memory-policy.js; tools/measure-browser-processes.ps1`; user acceptance requirement|P1|

## D — Privacy and security

| Requirement | State / gap | Code evidence / engineering reason | Lane |
|---|---|---|---|
|Native Ad Blocking|Present native/companion; selected toggle tests, per-site efficacy unverified|`src/protection/; src/security/; src/storage/; android/app/src/main/`; user acceptance requirement|P1|
|Tracker Blocking|Present rule-based filtering; not universal tracker protection|`src/protection/; src/security/; src/storage/; android/app/src/main/`; user acceptance requirement|P1|
|Per-Site Blocker Exceptions|Present native/companion controls; more real-site testing required|`src/protection/; src/security/; src/storage/; android/app/src/main/`; user acceptance requirement|P1|
|Anti-Phishing|Not verified for exact behavior; needs supported API/native design and functional acceptance|`src/protection/; src/security/; src/storage/; android/app/src/main/`; user acceptance requirement|P1|
|Safe Downloads|Partial prompts/filtering and no autoexecution; no complete reputation service certification|`src/protection/; src/security/; src/storage/; android/app/src/main/`; user acceptance requirement|P1|
|HTTPS-Only Mode|Not verified for exact behavior; needs supported API/native design and functional acceptance|`src/protection/; src/security/; src/storage/; android/app/src/main/`; user acceptance requirement|P1|
|Secure DNS|Not verified for exact behavior; needs supported API/native design and functional acceptance|`src/protection/; src/security/; src/storage/; android/app/src/main/`; user acceptance requirement|P1|
|Cookie Auto-Delete|Not verified for exact behavior; needs supported API/native design and functional acceptance|`src/protection/; src/security/; src/storage/; android/app/src/main/`; user acceptance requirement|P1|
|Delete History Per Site|Not verified for exact behavior; needs supported API/native design and functional acceptance|`src/protection/; src/security/; src/storage/; android/app/src/main/`; user acceptance requirement|P1|
|Anti-Fingerprinting|Not verified for exact behavior; needs supported API/native design and functional acceptance|`src/protection/; src/security/; src/storage/; android/app/src/main/`; user acceptance requirement|P1|
|Permission Dashboard|Not verified for exact behavior; needs supported API/native design and functional acceptance|`src/protection/; src/security/; src/storage/; android/app/src/main/`; user acceptance requirement|P1|
|Microphone/Camera Indicators|Not verified for exact behavior; needs supported API/native design and functional acceptance|`src/protection/; src/security/; src/storage/; android/app/src/main/`; user acceptance requirement|P1|
|Extension Permission Auditing|Not verified for exact behavior; needs supported API/native design and functional acceptance|`src/protection/; src/security/; src/storage/; android/app/src/main/`; user acceptance requirement|P1|
|Private Browsing|Chrome Runtime private UI expected; dedicated end-to-end leakage/cleanup acceptance pending|`src/protection/; src/security/; src/storage/; android/app/src/main/`; user acceptance requirement|P1|
|Temporary Profiles|Partial opt-in agent separate profile design; generic temporary browsing UX unverified|`src/protection/; src/security/; src/storage/; android/app/src/main/`; user acceptance requirement|P1|
|Secure Password Storage|Not verified for exact behavior; needs supported API/native design and functional acceptance|`src/protection/; src/security/; src/storage/; android/app/src/main/`; user acceptance requirement|P1|
|Passkeys|Not verified for exact behavior; needs supported API/native design and functional acceptance|`src/protection/; src/security/; src/storage/; android/app/src/main/`; user acceptance requirement|P1|
|Security Update Notifications|Not verified for exact behavior; needs supported API/native design and functional acceptance|`src/protection/; src/security/; src/storage/; android/app/src/main/`; user acceptance requirement|P0|
|Family Protection|Present DPAPI/Android encrypted policy; baseline native subset; new Android lifecycle pending|`src/protection/; src/security/; src/storage/; android/app/src/main/`; user acceptance requirement|P1|

## E — Extensions

| Requirement | State / gap | Code evidence / engineering reason | Lane |
|---|---|---|---|
|Manifest V3|Companion worker/popup baseline native CI subset; not arbitrary package certification|`src/extensions/extension_manager.cpp; src/app/maen_app.cpp; assets/companion/manifest.json`; user acceptance requirement|P1|
|Unpacked Extensions|Startup load-extension used; legacy facade rejects; user manager acceptance pending|`src/extensions/extension_manager.cpp; src/app/maen_app.cpp; assets/companion/manifest.json`; user acceptance requirement|P1|
|Extension Manager|Partial Chrome Runtime UI path; facade management unsupported, no fake buttons added|`src/extensions/extension_manager.cpp; src/app/maen_app.cpp; assets/companion/manifest.json`; user acceptance requirement|P1|
|Extension Updates|Not verified for exact behavior; needs supported API/native design and functional acceptance|`src/extensions/extension_manager.cpp; src/app/maen_app.cpp; assets/companion/manifest.json`; user acceptance requirement|P1|
|Permission Management|Not verified for exact behavior; needs supported API/native design and functional acceptance|`src/extensions/extension_manager.cpp; src/app/maen_app.cpp; assets/companion/manifest.json`; user acceptance requirement|P1|
|Extension Resource Monitor|Not verified for exact behavior; needs supported API/native design and functional acceptance|`src/extensions/extension_manager.cpp; src/app/maen_app.cpp; assets/companion/manifest.json`; user acceptance requirement|P1|
|Extension Profiles|Not verified for exact behavior; needs supported API/native design and functional acceptance|`src/extensions/extension_manager.cpp; src/app/maen_app.cpp; assets/companion/manifest.json`; user acceptance requirement|P1|
|Import Extension Settings|Not verified for exact behavior; needs supported API/native design and functional acceptance|`src/extensions/extension_manager.cpp; src/app/maen_app.cpp; assets/companion/manifest.json`; user acceptance requirement|P1|
|Compatibility Reports|Not verified for exact behavior; needs supported API/native design and functional acceptance|`src/extensions/extension_manager.cpp; src/app/maen_app.cpp; assets/companion/manifest.json`; user acceptance requirement|P1|
|Toolbar Extension Icons|Companion icon baseline popup evidence; third-party UI placement pending|`src/extensions/extension_manager.cpp; src/app/maen_app.cpp; assets/companion/manifest.json`; user acceptance requirement|P1|
|Extension Popups|Bundled companion ordinary-page popup baseline CI passed; third-party packages pending|`src/extensions/extension_manager.cpp; src/app/maen_app.cpp; assets/companion/manifest.json`; user acceptance requirement|P1|
|Extension Background Workers|Companion baseline CI subset; suspend/wake third-party fixtures pending|`src/extensions/extension_manager.cpp; src/app/maen_app.cpp; assets/companion/manifest.json`; user acceptance requirement|P1|
|Site-Specific Extension Activation|Not verified for exact behavior; needs supported API/native design and functional acceptance|`src/extensions/extension_manager.cpp; src/app/maen_app.cpp; assets/companion/manifest.json`; user acceptance requirement|P1|

## F — Media

| Requirement | State / gap | Code evidence / engineering reason | Lane |
|---|---|---|---|
|MP4|Baseline native codec/image fixture evidence; see exact scopes in MEDIA_COMPATIBILITY_MATRIX.md|`config/media-runtime.lock.json; src/media/; assets/companion/media-codecs.js`; user acceptance requirement|P1|
|WebM|Engine/OS dependent; actual codec/API/device workflow acceptance pending|`config/media-runtime.lock.json; src/media/; assets/companion/media-codecs.js`; user acceptance requirement|P1|
|H.264|Baseline native codec/image fixture evidence; see exact scopes in MEDIA_COMPATIBILITY_MATRIX.md|`config/media-runtime.lock.json; src/media/; assets/companion/media-codecs.js`; user acceptance requirement|P1|
|AAC|Baseline native codec/image fixture evidence; see exact scopes in MEDIA_COMPATIBILITY_MATRIX.md|`config/media-runtime.lock.json; src/media/; assets/companion/media-codecs.js`; user acceptance requirement|P1|
|AV1|Engine/OS dependent; actual codec/API/device workflow acceptance pending|`config/media-runtime.lock.json; src/media/; assets/companion/media-codecs.js`; user acceptance requirement|P1|
|VP9|Engine/OS dependent; actual codec/API/device workflow acceptance pending|`config/media-runtime.lock.json; src/media/; assets/companion/media-codecs.js`; user acceptance requirement|P1|
|VP8|Engine/OS dependent; actual codec/API/device workflow acceptance pending|`config/media-runtime.lock.json; src/media/; assets/companion/media-codecs.js`; user acceptance requirement|P1|
|Opus|Baseline native codec/image fixture evidence; see exact scopes in MEDIA_COMPATIBILITY_MATRIX.md|`config/media-runtime.lock.json; src/media/; assets/companion/media-codecs.js`; user acceptance requirement|P1|
|MP3|Engine/OS dependent; actual codec/API/device workflow acceptance pending|`config/media-runtime.lock.json; src/media/; assets/companion/media-codecs.js`; user acceptance requirement|P1|
|FLAC|Engine/OS dependent; actual codec/API/device workflow acceptance pending|`config/media-runtime.lock.json; src/media/; assets/companion/media-codecs.js`; user acceptance requirement|P1|
|WAV|Engine/OS dependent; actual codec/API/device workflow acceptance pending|`config/media-runtime.lock.json; src/media/; assets/companion/media-codecs.js`; user acceptance requirement|P1|
|Ogg|Engine/OS dependent; actual codec/API/device workflow acceptance pending|`config/media-runtime.lock.json; src/media/; assets/companion/media-codecs.js`; user acceptance requirement|P1|
|WebP|Baseline native codec/image fixture evidence; see exact scopes in MEDIA_COMPATIBILITY_MATRIX.md|`config/media-runtime.lock.json; src/media/; assets/companion/media-codecs.js`; user acceptance requirement|P1|
|AVIF|Baseline native codec/image fixture evidence; see exact scopes in MEDIA_COMPATIBILITY_MATRIX.md|`config/media-runtime.lock.json; src/media/; assets/companion/media-codecs.js`; user acceptance requirement|P1|
|GIF|Baseline native codec/image fixture evidence; see exact scopes in MEDIA_COMPATIBILITY_MATRIX.md|`config/media-runtime.lock.json; src/media/; assets/companion/media-codecs.js`; user acceptance requirement|P1|
|SVG|Baseline native codec/image fixture evidence; see exact scopes in MEDIA_COMPATIBILITY_MATRIX.md|`config/media-runtime.lock.json; src/media/; assets/companion/media-codecs.js`; user acceptance requirement|P1|
|PNG|Baseline native codec/image fixture evidence; see exact scopes in MEDIA_COMPATIBILITY_MATRIX.md|`config/media-runtime.lock.json; src/media/; assets/companion/media-codecs.js`; user acceptance requirement|P1|
|JPEG|Baseline native codec/image fixture evidence; see exact scopes in MEDIA_COMPATIBILITY_MATRIX.md|`config/media-runtime.lock.json; src/media/; assets/companion/media-codecs.js`; user acceptance requirement|P1|
|WebRTC|Engine/OS dependent; actual codec/API/device workflow acceptance pending|`config/media-runtime.lock.json; src/media/; assets/companion/media-codecs.js`; user acceptance requirement|P1|
|Web Audio|Engine/OS dependent; actual codec/API/device workflow acceptance pending|`config/media-runtime.lock.json; src/media/; assets/companion/media-codecs.js`; user acceptance requirement|P1|
|WebCodecs|Engine/OS dependent; actual codec/API/device workflow acceptance pending|`config/media-runtime.lock.json; src/media/; assets/companion/media-codecs.js`; user acceptance requirement|P1|
|Media Source Extensions|Engine/OS dependent; actual codec/API/device workflow acceptance pending|`config/media-runtime.lock.json; src/media/; assets/companion/media-codecs.js`; user acceptance requirement|P1|
|Picture-in-Picture|Engine/OS dependent; actual codec/API/device workflow acceptance pending|`config/media-runtime.lock.json; src/media/; assets/companion/media-codecs.js`; user acceptance requirement|P1|
|Fullscreen|Engine/OS dependent; actual codec/API/device workflow acceptance pending|`config/media-runtime.lock.json; src/media/; assets/companion/media-codecs.js`; user acceptance requirement|P1|
|Screen Sharing|Engine/OS dependent; actual codec/API/device workflow acceptance pending|`config/media-runtime.lock.json; src/media/; assets/companion/media-codecs.js`; user acceptance requirement|P1|
|Audio Device Selection|Engine/OS dependent; actual codec/API/device workflow acceptance pending|`config/media-runtime.lock.json; src/media/; assets/companion/media-codecs.js`; user acceptance requirement|P1|
|Bluetooth Headsets|Engine/OS dependent; actual codec/API/device workflow acceptance pending|`config/media-runtime.lock.json; src/media/; assets/companion/media-codecs.js`; user acceptance requirement|P1|
|Hardware Decoding|Engine/OS dependent; actual codec/API/device workflow acceptance pending|`config/media-runtime.lock.json; src/media/; assets/companion/media-codecs.js`; user acceptance requirement|P1|
|Software Fallback|Engine/OS dependent; actual codec/API/device workflow acceptance pending|`config/media-runtime.lock.json; src/media/; assets/companion/media-codecs.js`; user acceptance requirement|P1|
|Subtitles|Engine/OS dependent; actual codec/API/device workflow acceptance pending|`config/media-runtime.lock.json; src/media/; assets/companion/media-codecs.js`; user acceptance requirement|P1|
|Adaptive Streaming|Engine/OS dependent; actual codec/API/device workflow acceptance pending|`config/media-runtime.lock.json; src/media/; assets/companion/media-codecs.js`; user acceptance requirement|P1|
|Multiple Audio Tracks|Engine/OS dependent; actual codec/API/device workflow acceptance pending|`config/media-runtime.lock.json; src/media/; assets/companion/media-codecs.js`; user acceptance requirement|P1|
|Media Playback Diagnostics|Present companion diagnostics; sanitize exported URLs/tokens|`config/media-runtime.lock.json; src/media/; assets/companion/media-codecs.js`; user acceptance requirement|P1|
|DRM|Restricted: CDM/provider agreements and actual supported service integration required|`config/media-runtime.lock.json; src/media/; assets/companion/media-codecs.js`; user acceptance requirement|P1|
|HEVC|Restricted: build/platform/licensing dependent; not claimed supported|`config/media-runtime.lock.json; src/media/; assets/companion/media-codecs.js`; user acceptance requirement|P1|

## G — Sites and communication

| Requirement | State / gap | Code evidence / engineering reason | Lane |
|---|---|---|---|
|Facebook|No authenticated end-to-end test in this workspace; page load alone is insufficient|`tests/browser_smoke.mjs; src/app/maen_client.cpp; android/app/src/main/`; user acceptance requirement|P1|
|Instagram|No authenticated end-to-end test in this workspace; page load alone is insufficient|`tests/browser_smoke.mjs; src/app/maen_client.cpp; android/app/src/main/`; user acceptance requirement|P1|
|Messenger|No authenticated end-to-end test in this workspace; page load alone is insufficient|`tests/browser_smoke.mjs; src/app/maen_client.cpp; android/app/src/main/`; user acceptance requirement|P1|
|WhatsApp Web|No authenticated end-to-end test in this workspace; page load alone is insufficient|`tests/browser_smoke.mjs; src/app/maen_client.cpp; android/app/src/main/`; user acceptance requirement|P1|
|Telegram Web|No authenticated end-to-end test in this workspace; page load alone is insufficient|`tests/browser_smoke.mjs; src/app/maen_client.cpp; android/app/src/main/`; user acceptance requirement|P1|
|Discord|No authenticated end-to-end test in this workspace; page load alone is insufficient|`tests/browser_smoke.mjs; src/app/maen_client.cpp; android/app/src/main/`; user acceptance requirement|P1|
|Reddit|No authenticated end-to-end test in this workspace; page load alone is insufficient|`tests/browser_smoke.mjs; src/app/maen_client.cpp; android/app/src/main/`; user acceptance requirement|P1|
|X|No authenticated end-to-end test in this workspace; page load alone is insufficient|`tests/browser_smoke.mjs; src/app/maen_client.cpp; android/app/src/main/`; user acceptance requirement|P1|
|LinkedIn|No authenticated end-to-end test in this workspace; page load alone is insufficient|`tests/browser_smoke.mjs; src/app/maen_client.cpp; android/app/src/main/`; user acceptance requirement|P1|
|TikTok|No authenticated end-to-end test in this workspace; page load alone is insufficient|`tests/browser_smoke.mjs; src/app/maen_client.cpp; android/app/src/main/`; user acceptance requirement|P1|
|YouTube|No authenticated end-to-end test in this workspace; page load alone is insufficient|`tests/browser_smoke.mjs; src/app/maen_client.cpp; android/app/src/main/`; user acceptance requirement|P1|
|Gmail|No authenticated end-to-end test in this workspace; page load alone is insufficient|`tests/browser_smoke.mjs; src/app/maen_client.cpp; android/app/src/main/`; user acceptance requirement|P1|
|Outlook|No authenticated end-to-end test in this workspace; page load alone is insufficient|`tests/browser_smoke.mjs; src/app/maen_client.cpp; android/app/src/main/`; user acceptance requirement|P1|
|Microsoft 365|No authenticated end-to-end test in this workspace; page load alone is insufficient|`tests/browser_smoke.mjs; src/app/maen_client.cpp; android/app/src/main/`; user acceptance requirement|P1|
|Google Docs|No authenticated end-to-end test in this workspace; page load alone is insufficient|`tests/browser_smoke.mjs; src/app/maen_client.cpp; android/app/src/main/`; user acceptance requirement|P1|
|Google Meet|No authenticated end-to-end test in this workspace; page load alone is insufficient|`tests/browser_smoke.mjs; src/app/maen_client.cpp; android/app/src/main/`; user acceptance requirement|P1|
|Teams|No authenticated end-to-end test in this workspace; page load alone is insufficient|`tests/browser_smoke.mjs; src/app/maen_client.cpp; android/app/src/main/`; user acceptance requirement|P1|
|Zoom|No authenticated end-to-end test in this workspace; page load alone is insufficient|`tests/browser_smoke.mjs; src/app/maen_client.cpp; android/app/src/main/`; user acceptance requirement|P1|
|Slack|No authenticated end-to-end test in this workspace; page load alone is insufficient|`tests/browser_smoke.mjs; src/app/maen_client.cpp; android/app/src/main/`; user acceptance requirement|P1|
|Twitch|No authenticated end-to-end test in this workspace; page load alone is insufficient|`tests/browser_smoke.mjs; src/app/maen_client.cpp; android/app/src/main/`; user acceptance requirement|P1|
|Discourse|No authenticated end-to-end test in this workspace; page load alone is insufficient|`tests/browser_smoke.mjs; src/app/maen_client.cpp; android/app/src/main/`; user acceptance requirement|P1|
|phpBB|No authenticated end-to-end test in this workspace; page load alone is insufficient|`tests/browser_smoke.mjs; src/app/maen_client.cpp; android/app/src/main/`; user acceptance requirement|P1|
|XenForo|No authenticated end-to-end test in this workspace; page load alone is insufficient|`tests/browser_smoke.mjs; src/app/maen_client.cpp; android/app/src/main/`; user acceptance requirement|P1|

## H — Interface

| Requirement | State / gap | Code evidence / engineering reason | Lane |
|---|---|---|---|
|Custom Start Page|Present, compact clock/weather/search logic; installed visual acceptance pending|`assets/companion/start.html; ui.css; reader.js; start-dashboard.js; src/app/maen_app.cpp`; user acceptance requirement|P2|
|Themes|Not verified for exact behavior; needs supported API/native design and functional acceptance|`assets/companion/start.html; ui.css; reader.js; start-dashboard.js; src/app/maen_app.cpp`; user acceptance requirement|P2|
|Dark/Light Mode|Present companion theme styles; full native theme parity not certified|`assets/companion/start.html; ui.css; reader.js; start-dashboard.js; src/app/maen_app.cpp`; user acceptance requirement|P2|
|Dynamic UI Layout|Not verified for exact behavior; needs supported API/native design and functional acceptance|`assets/companion/start.html; ui.css; reader.js; start-dashboard.js; src/app/maen_app.cpp`; user acceptance requirement|P2|
|Movable Toolbars|Not verified for exact behavior; needs supported API/native design and functional acceptance|`assets/companion/start.html; ui.css; reader.js; start-dashboard.js; src/app/maen_app.cpp`; user acceptance requirement|P2|
|Sidebar|Not verified for exact behavior; needs supported API/native design and functional acceptance|`assets/companion/start.html; ui.css; reader.js; start-dashboard.js; src/app/maen_app.cpp`; user acceptance requirement|P2|
|Web Panels|Not verified for exact behavior; needs supported API/native design and functional acceptance|`assets/companion/start.html; ui.css; reader.js; start-dashboard.js; src/app/maen_app.cpp`; user acceptance requirement|P2|
|Adjustable Tab Bar|Not verified for exact behavior; needs supported API/native design and functional acceptance|`assets/companion/start.html; ui.css; reader.js; start-dashboard.js; src/app/maen_app.cpp`; user acceptance requirement|P2|
|Minimal Focus Mode|Not verified for exact behavior; needs supported API/native design and functional acceptance|`assets/companion/start.html; ui.css; reader.js; start-dashboard.js; src/app/maen_app.cpp`; user acceptance requirement|P2|
|Distraction-Free Reader|Present sanitized companion reader; real long-page/accessibility acceptance pending|`assets/companion/start.html; ui.css; reader.js; start-dashboard.js; src/app/maen_app.cpp`; user acceptance requirement|P2|
|Custom Search Engines|Present companion settings/search, portable tests|`assets/companion/start.html; ui.css; reader.js; start-dashboard.js; src/app/maen_app.cpp`; user acceptance requirement|P2|
|Address Bar Suggestions|Not verified for exact behavior; needs supported API/native design and functional acceptance|`assets/companion/start.html; ui.css; reader.js; start-dashboard.js; src/app/maen_app.cpp`; user acceptance requirement|P2|
|Bookmark Panels|Not verified for exact behavior; needs supported API/native design and functional acceptance|`assets/companion/start.html; ui.css; reader.js; start-dashboard.js; src/app/maen_app.cpp`; user acceptance requirement|P2|
|Quick Access|Present start-page links; color/contrast manual acceptance pending|`assets/companion/start.html; ui.css; reader.js; start-dashboard.js; src/app/maen_app.cpp`; user acceptance requirement|P2|
|Accessibility|Not verified for exact behavior; needs supported API/native design and functional acceptance|`assets/companion/start.html; ui.css; reader.js; start-dashboard.js; src/app/maen_app.cpp`; user acceptance requirement|P2|
|RTL Arabic Support|Present Arabic companion translations/RTL; native screen-reader/input acceptance pending|`assets/companion/start.html; ui.css; reader.js; start-dashboard.js; src/app/maen_app.cpp`; user acceptance requirement|P2|
|Font Scaling|Not verified for exact behavior; needs supported API/native design and functional acceptance|`assets/companion/start.html; ui.css; reader.js; start-dashboard.js; src/app/maen_app.cpp`; user acceptance requirement|P2|
|High Contrast|Partial companion CSS; assistive-tech/device tests pending|`assets/companion/start.html; ui.css; reader.js; start-dashboard.js; src/app/maen_app.cpp`; user acceptance requirement|P2|
|Touch-Friendly UI|Not verified for exact behavior; needs supported API/native design and functional acceptance|`assets/companion/start.html; ui.css; reader.js; start-dashboard.js; src/app/maen_app.cpp`; user acceptance requirement|P2|
|Customizable Shortcuts|Not verified for exact behavior; needs supported API/native design and functional acceptance|`assets/companion/start.html; ui.css; reader.js; start-dashboard.js; src/app/maen_app.cpp`; user acceptance requirement|P2|

## I — Downloads/files

| Requirement | State / gap | Code evidence / engineering reason | Lane |
|---|---|---|---|
|Resume Downloads|Not verified for exact behavior; needs supported API/native design and functional acceptance|`src/app/maen_client.cpp; assets/companion/downloads.js; android/app/src/main/`; user acceptance requirement|P1|
|Download Queue|Not verified for exact behavior; needs supported API/native design and functional acceptance|`src/app/maen_client.cpp; assets/companion/downloads.js; android/app/src/main/`; user acceptance requirement|P1|
|Download Speed/Progress|Partial companion/native progress display; real network/time correctness pending|`src/app/maen_client.cpp; assets/companion/downloads.js; android/app/src/main/`; user acceptance requirement|P1|
|Download Error Recovery|Not verified for exact behavior; needs supported API/native design and functional acceptance|`src/app/maen_client.cpp; assets/companion/downloads.js; android/app/src/main/`; user acceptance requirement|P1|
|Parallel Downloads|Not verified for exact behavior; needs supported API/native design and functional acceptance|`src/app/maen_client.cpp; assets/companion/downloads.js; android/app/src/main/`; user acceptance requirement|P1|
|Download Integrity Checks|Not verified for exact behavior; needs supported API/native design and functional acceptance|`src/app/maen_client.cpp; assets/companion/downloads.js; android/app/src/main/`; user acceptance requirement|P1|
|Safe File Names|Present Android safe bounded random-prefixed names; Windows runtime save flow|`src/app/maen_client.cpp; assets/companion/downloads.js; android/app/src/main/`; user acceptance requirement|P1|
|Download History|Present runtime/companion paths, installed acceptance pending|`src/app/maen_client.cpp; assets/companion/downloads.js; android/app/src/main/`; user acceptance requirement|P1|
|PDF Viewing|Runtime PDF path; installed desktop/mobile file/printing acceptance pending|`src/app/maen_client.cpp; assets/companion/downloads.js; android/app/src/main/`; user acceptance requirement|P1|
|Print Preview|Not verified for exact behavior; needs supported API/native design and functional acceptance|`src/app/maen_client.cpp; assets/companion/downloads.js; android/app/src/main/`; user acceptance requirement|P1|
|Image Viewer|Not verified for exact behavior; needs supported API/native design and functional acceptance|`src/app/maen_client.cpp; assets/companion/downloads.js; android/app/src/main/`; user acceptance requirement|P1|
|Save Page|Not verified for exact behavior; needs supported API/native design and functional acceptance|`src/app/maen_client.cpp; assets/companion/downloads.js; android/app/src/main/`; user acceptance requirement|P1|
|Screenshot|Not verified for exact behavior; needs supported API/native design and functional acceptance|`src/app/maen_client.cpp; assets/companion/downloads.js; android/app/src/main/`; user acceptance requirement|P1|
|File Uploads|Present native/WebView picker paths; account/site/permission tests pending|`src/app/maen_client.cpp; assets/companion/downloads.js; android/app/src/main/`; user acceptance requirement|P1|
|Drag and Drop|Not verified for exact behavior; needs supported API/native design and functional acceptance|`src/app/maen_client.cpp; assets/companion/downloads.js; android/app/src/main/`; user acceptance requirement|P1|
|GET attachment|Baseline Windows native download/content check passed; modified branch rerun pending|`src/app/maen_client.cpp; assets/companion/downloads.js; android/app/src/main/`; user acceptance requirement|P1|
|POST attachment|Baseline Windows native original body/referrer/content test passed; no replay|`src/app/maen_client.cpp; assets/companion/downloads.js; android/app/src/main/`; user acceptance requirement|P0|
|Redirected popup attachment|Baseline Windows native redirect/referrer/content test passed|`src/app/maen_client.cpp; assets/companion/downloads.js; android/app/src/main/`; user acceptance requirement|P0|

## J — Mobile/sync

| Requirement | State / gap | Code evidence / engineering reason | Lane |
|---|---|---|---|
|Desktop/Android Sync|Absent sync transport/pairing; local export is not synchronization|`android/app/src/main/; assets/companion/session-restore.js`; user acceptance requirement|P2|
|Encrypted Bookmark Sync|Absent sync transport/pairing; local export is not synchronization|`android/app/src/main/; assets/companion/session-restore.js`; user acceptance requirement|P2|
|Tab Sync|Absent sync transport/pairing; local export is not synchronization|`android/app/src/main/; assets/companion/session-restore.js`; user acceptance requirement|P2|
|Send Tab to Device|Absent sync transport/pairing; local export is not synchronization|`android/app/src/main/; assets/companion/session-restore.js`; user acceptance requirement|P2|
|Encrypted Settings Sync|Absent sync transport/pairing; local export is not synchronization|`android/app/src/main/; assets/companion/session-restore.js`; user acceptance requirement|P2|
|QR Pairing|Absent sync transport/pairing; local export is not synchronization|`android/app/src/main/; assets/companion/session-restore.js`; user acceptance requirement|P2|
|Android Download Improvements|Partial DownloadManager GET; no universal authenticated POST/blob handling|`android/app/src/main/; assets/companion/session-restore.js`; user acceptance requirement|P2|
|Mobile Tab Groups|Not verified for exact behavior; needs supported API/native design and functional acceptance|`android/app/src/main/; assets/companion/session-restore.js`; user acceptance requirement|P2|
|Toolbar Customization|Not verified for exact behavior; needs supported API/native design and functional acceptance|`android/app/src/main/; assets/companion/session-restore.js`; user acceptance requirement|P2|
|Mobile Reader Mode|Not verified for exact behavior; needs supported API/native design and functional acceptance|`android/app/src/main/; assets/companion/session-restore.js`; user acceptance requirement|P2|
|Battery-Saving Mode|Not verified for exact behavior; needs supported API/native design and functional acceptance|`android/app/src/main/; assets/companion/session-restore.js`; user acceptance requirement|P2|
|Password Manager Integration|Not verified for exact behavior; needs supported API/native design and functional acceptance|`android/app/src/main/; assets/companion/session-restore.js`; user acceptance requirement|P2|
|PWA|Not verified for exact behavior; needs supported API/native design and functional acceptance|`android/app/src/main/; assets/companion/session-restore.js`; user acceptance requirement|P2|
|Notification Compatibility|Partial desktop runtime API; Android host implementation not verified|`android/app/src/main/; assets/companion/session-restore.js`; user acceptance requirement|P2|

## Community refinements and priorities

Each original request is linked. The CSV contains actual counts, estimates, source OS limitations and the score rubric; P0 describes critical workflow risk, not proof a competitor bug reproduces here.

| Request | Evidence | MaenBrowser state | Lane | Weighted score |
|---|---|---|---|---|
|CR-001 `add-a-download-button-to-vivaldi-s-built-in-pdf-viewer`|[original](https://forum.vivaldi.net/topic/122079/add-a-download-button-to-vivaldi-s-built-in-pdf-viewer)|Not verified for this exact refinement|P2|42.0|
|CR-002 `add-advanced-pdf-reader-functionality-highlight-draw-annotate-similar-to-microsoft-edge`|[original](https://github.com/brave/brave-browser/issues/45723)|Not verified for this exact refinement|P3|38.0|
|CR-003 `add-an-option-to-hide-the-new-tab-button`|[original](https://github.com/brave/brave-browser/issues/43839)|Not verified for this exact refinement|P2|46.0|
|CR-004 `add-an-option-to-remove-the-close-button-in-the-bottom-right-corner-in-a-split-view`|[original](https://community.brave.app/t/add-an-option-to-remove-the-close-button-in-the-bottom-right-corner-in-a-split-view/659459)|Not verified for this exact refinement|P2|42.0|
|CR-005 `add-history-to-sidebar`|[original](https://community.brave.app/t/add-history-to-sidebar/396212)|Not verified for this exact refinement|P2|39.5|
|CR-006 `add-list-and-stack-tab-types-to-the-brave-browser-in-addition-to-the-existing-grid-tab-type`|[original](https://community.brave.app/t/add-list-and-stack-tab-types-to-the-brave-browser-in-addition-to-the-existing-grid-tab-type/657765)|Not verified for this exact refinement|P2|47.0|
|CR-007 `add-reading-list-feature-to-brave-mobile`|[original](https://github.com/brave/brave-browser/issues/44606)|Not verified for this exact refinement|P3|38.0|
|CR-008 `add-site-exceptions-for-clear-data-on-close`|[original](https://community.brave.app/t/add-site-exceptions-for-clear-data-on-close/647201)|Not verified for this exact refinement|P3|38.0|
|CR-009 `add-support-for-browsing-profiles-on-mobile`|[original](https://github.com/brave/brave-browser/issues/45907)|Not verified for this exact refinement|P1|52.0|
|CR-010 `add-tab-to-closed-tab-group-without-loading-all-tabs`|[original](https://github.com/brave/brave-browser/issues/49911)|Not verified for this exact refinement|P2|47.0|
|CR-011 `adding-copy-clean-link-to-about-keyboard-shortcuts`|[original](https://connect.mozilla.org/t5/ideas/adding-quot-copy-clean-link-quot-to-about-keyboard-shortcuts/idi-p/145144)|Not verified for this exact refinement|P2|42.0|
|CR-012 `allow-users-to-customize-the-browser-header-color`|[original](https://community.brave.app/t/allow-users-to-customize-the-browser-header-color/656859)|Not verified for this exact refinement|P2|47.0|
|CR-013 `alphabetical-and-manual-sorting-for-tab-groups-and-containers`|[original](https://community.brave.app/t/alphabetical-and-manual-sorting-for-tab-groups-and-containers/660306)|Not verified for this exact refinement|P1|56.0|
|CR-014 `android-tab-stack-option-to-remove-close-button-x`|[original](https://forum.vivaldi.net/topic/121777/android-tab-stack-option-to-remove-close-button-x)|Not verified for this exact refinement|P2|46.0|
|CR-015 `android-webcontent-dark-mode-toggle`|[original](https://www.reddit.com/r/brave_browser/comments/165ppgf/dark_mode_toggle_on_android/)|Not verified for this exact refinement|P3|35.5|
|CR-016 `android-widgets-to-help-replace-chrome`|[original](https://forum.vivaldi.net/topic/121812/android-widgets-to-help-replace-chrome)|Not verified for this exact refinement|P3|38.0|
|CR-017 `auto-detect-logos-select-part-of-the-page-for-speed-dial-and-bookmark-thumbnails`|[original](https://forum.vivaldi.net/topic/24867/auto-detect-logos-select-part-of-the-page-for-speed-dial-and-bookmark-thumbnails)|Not verified for this exact refinement|P2|39.5|
|CR-018 `auto-hide-panels-should-require-cursor-to-hit-a-defined-hotzone-not-just-proximity-to-the-screen-edge`|[original](https://forum.vivaldi.net/topic/120293/auto-hide-panels-should-require-cursor-to-hit-a-defined-hotzone-not-just-proximity-to-the-screen-edge)|Not verified for this exact refinement|P3|38.0|
|CR-019 `automatic-container-site-rules`|[original](https://community.brave.app/t/suggestion-for-containers/659127)|Not verified for this exact refinement|P1|57.0|
|CR-020 `automatic-stack-by-host`|[original](https://forum.vivaldi.net/topic/121690/why-isn-t-there-an-automatic-tab-stacking-by-host-feature)|Not verified for this exact refinement|P3|38.0|
|CR-021 `automatic-tab-unload-reliability`|[original](https://github.com/zen-browser/desktop/issues/8822)|Not verified for this exact refinement|P1|59.0|
|CR-022 `automatically-delete-website-history-when-closing-its-last-tab-with-customizable-website-list`|[original](https://community.brave.app/t/automatically-delete-website-history-when-closing-its-last-tab-with-customizable-website-list/660610)|Not verified for this exact refinement|P2|47.0|
|CR-023 `automatically-share-link-to-device-if-only-one-device-is-avaliable`|[original](https://connect.mozilla.org/t5/ideas/automatically-share-link-to-device-if-only-one-device-is/idi-p/144997)|Not verified for this exact refinement|P3|38.0|
|CR-024 `back-to-pinned-url-feature-on-pinned-tabs`|[original](https://forum.vivaldi.net/topic/113487/reset-pinned-tabs)|Not verified for this exact refinement|P2|52.0|
|CR-025 `block-popups-tabs-before-they-try-to-open`|[original](https://community.brave.app/t/block-popups-tabs-before-they-try-to-open/658737)|Not verified for this exact refinement|P2|47.0|
|CR-026 `block-shorts-reels-doomscrolling`|[original](https://forum.vivaldi.net/topic/105142/block-shorts-reels-doomscrolling)|Not verified for this exact refinement|P3|38.0|
|CR-027 `bookmark-tags`|[original](https://news.ycombinator.com/item?id=39247432)|Not verified for this exact refinement|P3|35.5|
|CR-028 `brave-browser-named-windows-don-t-survive-a-restart-and-window-tabs-restore`|[original](https://community.brave.app/t/brave-browser-named-windows-dont-survive-a-restart-and-window-tabs-restore/659931)|Not verified for this exact refinement|P0|67.0|
|CR-029 `browser-profile-lock`|[original](https://community.brave.app/t/latch-a-profile-lock-for-brave-with-fingerprint-lock-unlock/660418)|Not verified for this exact refinement|P1|57.0|
|CR-030 `built-in-support-for-userstyles-usercss`|[original](https://forum.vivaldi.net/topic/24156/built-in-support-for-userstyles-usercss)|Not verified for this exact refinement|P3|25.5|
|CR-031 `collapse-folders-when-adding-a-bookmark`|[original](https://forum.vivaldi.net/topic/26593/collapse-folders-when-adding-a-bookmark)|Not verified for this exact refinement|P3|35.5|
|CR-032 `compact-context-menus`|[original](https://github.com/ungoogled-software/ungoogled-chromium/issues/3196)|Not verified for this exact refinement|P2|42.0|
|CR-033 `completely-disable-wallet-rewards-leo-and-other-crypto-features-in-brave-by-default`|[original](https://github.com/brave/brave-browser/issues/43030)|Not verified for this exact refinement|P3|38.0|
|CR-034 `confirmation-dialog-before-opening-email-links-to-websites`|[original](https://connect.mozilla.org/t5/ideas/confirmation-dialog-before-opening-email-links-to-websites/idi-p/144947)|Not verified for this exact refinement|P3|38.0|
|CR-035 `create-a-brave-flag-entry-so-that-scroll-to-text-fragment-can-be-disabled`|[original](https://github.com/brave/brave-browser/issues/42393)|Not verified for this exact refinement|P3|38.0|
|CR-036 `custom-search-engines`|[original](https://github.com/ungoogled-software/ungoogled-chromium/issues/3556)|Present; targeted portable tests, actual UI acceptance still required|P3|38.0|
|CR-037 `customizable-menu-items`|[original](https://forum.vivaldi.net/topic/68161/customizable-menu-items.)|Not verified for this exact refinement|P2|39.5|
|CR-038 `default-fallback-open-every-unassigned-navigation-in-a-new-temporary-container`|[original](https://community.brave.app/t/default-fallback-open-every-unassigned-navigation-in-a-new-temporary-container/660594)|Not verified for this exact refinement|P1|52.0|
|CR-039 `define-in-android-version-of-the-browser-opens-google`|[original](https://community.brave.app/t/define-in-android-version-of-the-browser-opens-google/660237)|Not verified for this exact refinement|P3|38.0|
|CR-040 `desktop-style-widgets-on-the-new-tab-screen`|[original](https://community.brave.app/t/desktop-style-widgets-on-the-new-tab-screen/651455)|Not verified for this exact refinement|P2|47.0|
|CR-041 `disable-split-view`|[original](https://github.com/brave/brave-browser/issues/52862)|Not verified for this exact refinement|P3|38.0|
|CR-042 `document-browser-threat-model`|[original](https://github.com/ungoogled-software/ungoogled-chromium/issues/3287)|Not verified for this exact refinement|P3|38.0|
|CR-043 `dynamic-split-screen`|[original](https://community.brave.app/t/dynamic-split-screen/659212)|Not verified for this exact refinement|P3|38.0|
|CR-044 `expandable-on-hover-toolbar-needed`|[original](https://connect.mozilla.org/t5/ideas/quot-expandable-on-hover-quot-toolbar-needed/idi-p/144896)|Not verified for this exact refinement|P2|42.0|
|CR-045 `extension-compatibility-report`|[original](https://github.com/ungoogled-software/ungoogled-chromium/issues/3565)|Not verified for this exact refinement|P1|59.0|
|CR-046 `feature-request-add-a-toggle-button-on-picture-in-picture-window-for-hidden-hover-mode`|[original](https://forum.vivaldi.net/topic/121964/feature-request-add-a-toggle-button-on-picture-in-picture-window-for-hidden-hover-mode)|Not verified for this exact refinement|P2|42.0|
|CR-047 `feature-request-add-an-optional-right-click-to-close-tabs-setting`|[original](https://community.brave.app/t/feature-request-add-an-optional-right-click-to-close-tabs-setting/660425)|Not verified for this exact refinement|P2|47.0|
|CR-048 `feature-request-add-modify-remove-http-request-headers-in-mobile-browser`|[original](https://forum.vivaldi.net/topic/121971/feature-request-add-modify-remove-http-request-headers-in-mobile-browser)|Not verified for this exact refinement|P3|28.0|
|CR-049 `feature-request-open-tabs-directly-from-collapsed-tab-groups`|[original](https://community.brave.app/t/feature-request-open-tabs-directly-from-collapsed-tab-groups/659345)|Not verified for this exact refinement|P2|47.0|
|CR-050 `feature-request-show-update-date-time-of-latest-build-on-the-about-brave-page`|[original](https://community.brave.app/t/feature-request-show-update-date-time-of-latest-build-on-the-about-brave-page/659822)|Not verified for this exact refinement|P3|38.0|
|CR-051 `feature-request-transparency-option-for-speed-dial-background-pads-and-option-to-hide-site-names`|[original](https://forum.vivaldi.net/topic/122029/feature-request-transparency-option-for-speed-dial-background-pads-and-option-to-hide-site-names)|Not verified for this exact refinement|P2|42.0|
|CR-052 `feature-request-video-assistant-swipe-gestures-for-volume-and-brightness-like-samsung-internet`|[original](https://community.brave.app/t/feature-request-video-assistant-swipe-gestures-for-volume-and-brightness-like-samsung-internet/657325)|Not verified for this exact refinement|P3|38.0|
|CR-053 `fix-the-bug-with-the-black-screen-at-the-bottom-it-s-been-like-a-month-and-it-s-still-there-all-you-care-is-brave-origin-and-money`|[original](https://community.brave.app/t/fix-the-bug-with-the-black-screen-at-the-bottom-its-been-like-a-month-and-its-still-there-all-you-care-is-brave-origin-and-money/657057)|Not verified for this exact refinement|P0|67.0|
|CR-054 `focus-mode-hide-browser-bars`|[original](https://community.brave.app/t/ability-to-hide-url-top-bar/659239)|Not verified for this exact refinement|P3|43.0|
|CR-055 `full-page-screenshot`|[original](https://community.brave.app/t/full-page-screenshot/658388)|Not verified for this exact refinement|P3|38.0|
|CR-056 `having-firefox-remember-tabs-from-containers`|[original](https://connect.mozilla.org/t5/ideas/having-firefox-remember-tabs-from-containers/idi-p/145181)|Not verified for this exact refinement|P1|56.0|
|CR-057 `hide-vertical-tabs-delay`|[original](https://community.brave.app/t/vertical-tabs-increase-max-delay-well-beyond-400ms/660486)|Not verified for this exact refinement|P2|57.0|
|CR-058 `history-timeline`|[original](https://news.ycombinator.com/item?id=39247432)|Not verified for this exact refinement|P3|35.5|
|CR-059 `horizontal-vertical-choice`|[original](https://www.reddit.com/r/browsers/comments/1iuwj21/)|Not verified for this exact refinement|P3|38.0|
|CR-060 `idle-cpu-and-energy-use`|[original](https://github.com/zen-browser/desktop/issues/8428)|Not verified for this exact refinement|P1|59.0|
|CR-061 `implement-an-optional-hevc-h-265-codec-component-for-firefox`|[original](https://connect.mozilla.org/t5/ideas/implement-an-optional-hevc-h-265-codec-component-for-firefox/idi-p/145063)|Not verified for this exact refinement|P3|38.0|
|CR-062 `implement-custom-scriptlets-in-brave-mobile`|[original](https://github.com/brave/brave-browser/issues/43105)|Not verified for this exact refinement|P3|28.0|
|CR-063 `import-export-settings-preferences-for-faster-setup`|[original](https://github.com/brave/brave-browser/issues/46192)|Not verified for this exact refinement|P3|38.0|
|CR-064 `improve-web-app-installation-management-and-icons-in-brave`|[original](https://community.brave.app/t/improve-web-app-installation-management-and-icons-in-brave/659437)|Not verified for this exact refinement|P3|38.0|
|CR-065 `incognito-mode-light-and-different-tab-style`|[original](https://community.brave.app/t/incognito-mode-light-and-different-tab-style/660526)|Not verified for this exact refinement|P2|47.0|
|CR-066 `keep-new-tab-search-bar-results-in-the-middle-of-the-screen-prevent-jump-to-address-bar`|[original](https://connect.mozilla.org/t5/ideas/keep-new-tab-search-bar-results-in-the-middle-of-the-screen/idi-p/144849)|Not verified for this exact refinement|P2|47.0|
|CR-067 `let-us-set-background-color-on-a-new-tab-to-white`|[original](https://community.brave.app/t/let-us-set-background-color-on-a-new-tab-to-white/660244)|Not verified for this exact refinement|P2|46.0|
|CR-068 `login-cookie-durability`|[original](https://github.com/zen-browser/desktop/issues/7212)|Not verified for this exact refinement|P0|67.0|
|CR-069 `long-press-to-copy-url`|[original](https://community.brave.app/t/long-press-to-copy-url/657427)|Not verified for this exact refinement|P3|38.0|
|CR-070 `long-press-to-open-bookmarks-and-speed-dials-in-background-tabs-on-android`|[original](https://forum.vivaldi.net/topic/122069/long-press-to-open-bookmarks-and-speed-dials-in-background-tabs-on-android)|Not verified for this exact refinement|P2|46.0|
|CR-071 `lts-ui-looks-option`|[original](https://forum.vivaldi.net/topic/122097/lts-ui-looks-option.)|Not verified for this exact refinement|P3|38.0|
|CR-072 `make-sync-server-url-configurable-in-brave-flags`|[original](https://github.com/brave/brave-browser/issues/43181)|No sync transport identified; local session export is not sync|P2|52.0|
|CR-073 `make-tabs-look-like-tabs-again`|[original](https://forum.vivaldi.net/topic/122057/make-tabs-look-like-tabs-again)|Not verified for this exact refinement|P2|47.0|
|CR-074 `manual-tab-hibernation`|[original](https://community.brave.app/t/add-option-to-manually-unload-tab/659570)|Partial: discard policy guards; requested context-menu refinement unverified|P2|57.0|
|CR-075 `misplacement-of-the-new-tab-button-after-moving-the-address-bar-to-the-bottom-on-brave-android-v1-90-122`|[original](https://community.brave.app/t/misplacement-of-the-new-tab-button-after-moving-the-address-bar-to-the-bottom-on-brave-android-v1-90-122/653258)|Not verified for this exact refinement|P2|46.0|
|CR-076 `mobile-desktop-sync`|[original](https://www.reddit.com/r/browsers/comments/1iuwj21/)|No sync transport identified; local session export is not sync|P2|52.0|
|CR-077 `mobile-export-bookmarks`|[original](https://community.brave.app/t/bookmark-export-issue/617526)|Not verified for this exact refinement|P3|38.0|
|CR-078 `modernize-bookmark-management-add-search-breadcrumbs-notes`|[original](https://connect.mozilla.org/t5/ideas/modernize-bookmark-management-add-search-breadcrumbs-amp-notes/idi-p/145135)|Not verified for this exact refinement|P3|38.0|
|CR-079 `move-vertical-sidebar-to-the-left-right-when-using-firefox-in-split-screen`|[original](https://connect.mozilla.org/t5/ideas/move-vertical-sidebar-to-the-left-right-when-using-firefox-in/idi-p/145126)|Not verified for this exact refinement|P2|42.0|
|CR-080 `native-ad-blocking`|[original](https://www.reddit.com/r/browsers/comments/1iuwj21/)|Present; native + companion policies, website-specific efficacy unverified|P3|48.0|
|CR-081 `nesting-grouped-tabs`|[original](https://community.brave.app/t/nesting-grouped-tabs/660561)|Not verified for this exact refinement|P2|47.0|
|CR-082 `new-no-history-tab-window-option`|[original](https://community.brave.app/t/new-no-history-tab-window-option/659149)|Not verified for this exact refinement|P2|47.0|
|CR-083 `new-tab-has-been-moved-can-it-go-back`|[original](https://github.com/brave/brave-browser/issues/43801)|Not verified for this exact refinement|P2|47.0|
|CR-084 `open-bookmark-folder-in-tab-stack`|[original](https://forum.vivaldi.net/topic/24578/open-bookmark-folder-in-tab-stack)|Not verified for this exact refinement|P2|44.5|
|CR-085 `option-for-transparent-status-bar-edge-to-edge-layout-fullscreen-during-normal-browsing`|[original](https://community.brave.app/t/option-for-transparent-status-bar-edge-to-edge-layout-fullscreen-during-normal-browsing/657531)|Not verified for this exact refinement|P3|38.0|
|CR-086 `option-to-customize-bottom-toolbar-layout-move-home-button-to-the-right`|[original](https://community.brave.app/t/option-to-customize-bottom-toolbar-layout-move-home-button-to-the-right/660412)|Not verified for this exact refinement|P2|47.0|
|CR-087 `option-to-have-the-edit-bookmark-button-actually-edit-the-bookmark-instead-of-just-removing-it`|[original](https://community.brave.app/t/option-to-have-the-edit-bookmark-button-actually-edit-the-bookmark-instead-of-just-removing-it/259250)|Not verified for this exact refinement|P2|39.5|
|CR-088 `option-to-hibernate-pinned-tabs-instead-of-closing`|[original](https://forum.vivaldi.net/topic/102080/option-to-hibernate-pinned-tabs-instead-of-closing)|Not verified for this exact refinement|P2|44.5|
|CR-089 `option-to-make-copy-sanitized-link-the-default`|[original](https://forum.vivaldi.net/topic/122089/option-to-make-copy-sanitized-link-the-default)|Not verified for this exact refinement|P3|38.0|
|CR-090 `option-to-remove-extensions-from-the-tab-bar`|[original](https://forum.vivaldi.net/topic/122007/option-to-remove-extensions-from-the-tab-bar)|Not verified for this exact refinement|P2|47.0|
|CR-091 `optional-heavy-features`|[original](https://www.reddit.com/r/firefox/comments/1n6jtyl/)|Present resource modes; lightness on real devices not yet measured|P3|38.0|
|CR-092 `parking-lot-for-tabs-that-are-already-athenticated-and-used-from-time-to-time-so-that-not-so-many-tabs-are-always-open`|[original](https://connect.mozilla.org/t5/ideas/parking-lot-for-tabs-that-are-already-athenticated-and-used-from/idi-p/144852)|Not verified for this exact refinement|P2|47.0|
|CR-093 `password-generator-for-brave`|[original](https://community.brave.app/t/password-generator-for-brave/659020)|Not verified for this exact refinement|P3|48.0|
|CR-094 `pinned-tabs`|[original](https://www.reddit.com/r/browsers/comments/1iuwj21/)|Chrome Runtime tab APIs used; persistence needs native acceptance|P2|47.0|
|CR-095 `please-add-an-option-to-disable-automatic-zoom-when-focusing-form-fields`|[original](https://forum.vivaldi.net/topic/121226/please-add-an-option-to-disable-automatic-zoom-when-focusing-form-fields)|Not verified for this exact refinement|P3|38.0|
|CR-096 `preserve-sessions-during-upgrade`|[original](https://github.com/zen-browser/desktop/issues/11994)|Not verified for this exact refinement|P0|67.0|
|CR-097 `privacy-security-settings-confusing-redundant-misleading`|[original](https://connect.mozilla.org/t5/ideas/privacy-amp-security-settings-confusing-redundant-amp-misleading/idi-p/144932)|Not verified for this exact refinement|P3|38.0|
|CR-098 `profiles-dialog-shortcut-key-for-each-profile-to-launch`|[original](https://forum.vivaldi.net/topic/121879/profiles-dialog-shortcut-key-for-each-profile-to-launch)|Not verified for this exact refinement|P2|54.0|
|CR-099 `quick-commands-show-the-url-for-history-results-like-tabs-and-bookmarks-already-do`|[original](https://forum.vivaldi.net/topic/121927/quick-commands-show-the-url-for-history-results-like-tabs-and-bookmarks-already-do)|Not verified for this exact refinement|P2|47.0|
|CR-100 `read-aloud`|[original](https://community.brave.app/t/can-you-add-an-android-tts-like-edge-has/657041)|Not identified in current app|P3|48.0|
|CR-101 `register-as-a-xml-handler-on-windows`|[original](https://forum.vivaldi.net/topic/122019/register-as-a-.xml-handler-on-windows)|Not verified for this exact refinement|P3|38.0|
|CR-102 `remember-last-used-bookmark-location-after-bookmarking-with-bookmark-link-context-menu-item`|[original](https://connect.mozilla.org/t5/ideas/remember-last-used-bookmark-location-after-bookmarking-with/idi-p/145043)|Not verified for this exact refinement|P2|42.0|
|CR-103 `restore-browser-session`|[original](https://community.brave.app/t/restore-browser-session/659713)|Partial: saved URL/group snapshots and deferred restore; not full unsaved DOM state|P2|57.0|
|CR-104 `restore-the-old-translation-option`|[original](https://connect.mozilla.org/t5/ideas/restore-the-old-translation-option/idi-p/145088)|Not verified for this exact refinement|P3|38.0|
|CR-105 `retain-tab-and-session-data-for-short-period-after-closing-firefox`|[original](https://connect.mozilla.org/t5/ideas/retain-tab-and-session-data-for-short-period-after-closing/idi-p/145112)|Not verified for this exact refinement|P2|47.0|
|CR-106 `save-as-pdf-feature-for-web-pages`|[original](https://community.brave.app/t/save-as-pdf-feature-for-web-pages/657027)|Not verified for this exact refinement|P3|38.0|
|CR-107 `save-present-window-as-profile`|[original](https://community.brave.app/t/save-present-window-as-profile/660592)|Not verified for this exact refinement|P1|52.0|
|CR-108 `save-tab-groups-to-sidebar-with-auto-hide-support-for-clutter-reduction-and-ram-efficiency`|[original](https://community.brave.app/t/save-tab-groups-to-sidebar-with-auto-hide-support-for-clutter-reduction-and-ram-efficiency/656891)|Not verified for this exact refinement|P2|46.0|
|CR-109 `self-hosted-sync`|[original](https://news.ycombinator.com/item?id=39247432)|No sync transport identified; local session export is not sync|P2|49.5|
|CR-110 `separated-private-window-on-windows-taskbar`|[original](https://forum.vivaldi.net/topic/122058/separated-private-window-on-windows-taskbar)|Not verified for this exact refinement|P3|38.0|
|CR-111 `separator-separating-pinned-and-pinless-tabs`|[original](https://connect.mozilla.org/t5/ideas/separator-separating-pinned-and-pinless-tabs/idi-p/144848)|Not verified for this exact refinement|P2|47.0|
|CR-112 `settings-theme-accent-color-should-offer-material-you-option`|[original](https://forum.vivaldi.net/topic/79864/settings-theme-accent-color-should-offer-material-you-option)|Not verified for this exact refinement|P2|39.5|
|CR-113 `share-button-to-open-windows-share-sheet-directly`|[original](https://connect.mozilla.org/t5/ideas/share-button-to-open-windows-share-sheet-directly/idi-p/145187)|Not verified for this exact refinement|P2|42.0|
|CR-114 `shortcut-to-open-the-settings`|[original](https://community.brave.app/t/shortcut-to-open-the-settings/467089)|Not verified for this exact refinement|P2|39.5|
|CR-115 `speed-dials-placement`|[original](https://community.brave.app/t/speed-dials-placement/657209)|Not verified for this exact refinement|P2|52.0|
|CR-116 `split-tabs-in-the-essential-arease`|[original](https://connect.mozilla.org/t5/ideas/split-tabs-in-the-essential-arease/idi-p/144917)|Not verified for this exact refinement|P2|47.0|
|CR-117 `split-view`|[original](https://www.reddit.com/r/browsers/comments/1iuwj21/)|Native implementation not identified; link organizer is not split rendering|P3|38.0|
|CR-118 `split-view-tool-bar-button`|[original](https://connect.mozilla.org/t5/ideas/split-view-tool-bar-button/idi-p/144834)|Not verified for this exact refinement|P2|42.0|
|CR-119 `stop-saving-tab-groups-automatically`|[original](https://github.com/brave/brave-browser/issues/45676)|Not verified for this exact refinement|P2|47.0|
|CR-120 `subscribe-to-scriptlets`|[original](https://github.com/brave/brave-browser/issues/43916)|Not verified for this exact refinement|P3|28.0|
|CR-121 `suggestion-to-be-able-to-change-themes-for-different-workspaces`|[original](https://forum.vivaldi.net/topic/110282/suggestion-to-be-able-to-change-themes-for-different-workspaces)|Not verified for this exact refinement|P2|46.0|
|CR-122 `suggestions-for-ui-improvements-on-the-new-tab-page`|[original](https://connect.mozilla.org/t5/ideas/suggestions-for-ui-improvements-on-the-quot-new-tab-quot-page/idi-p/145277)|Not verified for this exact refinement|P2|47.0|
|CR-123 `support-for-external-download-managers-on-android`|[original](https://community.brave.app/t/support-for-external-download-managers-on-android/656473)|Not verified for this exact refinement|P3|38.0|
|CR-124 `support-jpeg-xl`|[original](https://connect.mozilla.org/t5/ideas/support-jpeg-xl/idi-p/18433)|Not verified for this exact refinement|P3|35.5|
|CR-125 `support-tab-and-system-audio-sharing-during-video-calls`|[original](https://connect.mozilla.org/t5/ideas/support-tab-and-system-audio-sharing-during-video-calls/idi-p/144960)|Not verified for this exact refinement|P2|47.0|
|CR-126 `sync-workspaces-with-tabs-between-computers`|[original](https://forum.vivaldi.net/topic/100573/sync-workspaces-with-tabs-between-computers)|No sync transport identified; local session export is not sync|P2|53.5|
|CR-127 `tab-context-menu-stack`|[original](https://forum.vivaldi.net/topic/113324/right-click-to-move-a-tab-into-a-stack)|Not verified for this exact refinement|P2|51.0|
|CR-128 `tab-full-text-search`|[original](https://news.ycombinator.com/item?id=39247432)|Not verified for this exact refinement|P2|44.5|
|CR-129 `tab-stacks-with-1-tab`|[original](https://forum.vivaldi.net/topic/122015/tab-stacks-with-1-tab)|Not verified for this exact refinement|P2|47.0|
|CR-130 `tab-tree-hierarchy`|[original](https://forum.vivaldi.net/topic/24608/implement-tree-style-tabs)|Not verified for this exact refinement|P2|44.5|
|CR-131 `there-is-no-scrollbar-for-a-large-number-of-bookmarks`|[original](https://forum.vivaldi.net/topic/121950/there-is-no-scrollbar-for-a-large-number-of-bookmarks.)|Not verified for this exact refinement|P3|38.0|
|CR-132 `true-black-amoled-theme-for-firefox-android`|[original](https://connect.mozilla.org/t5/ideas/true-black-amoled-theme-for-firefox-android/idi-p/145161)|Not verified for this exact refinement|P2|47.0|
|CR-133 `ultra-dark-theme-contrast-issues-on-windows-proposal-for-improved-vertical-tabs-layout`|[original](https://github.com/brave/brave-browser/issues/52389)|Not verified for this exact refinement|P2|46.0|
|CR-134 `unable-to-delete-history-via-the-delete-key`|[original](https://connect.mozilla.org/t5/ideas/unable-to-delete-history-via-the-delete-key/idi-p/144862)|Not verified for this exact refinement|P3|38.0|
|CR-135 `vertical-tabs`|[original](https://www.reddit.com/r/browsers/comments/1iuwj21/)|Companion tab manager exists; native vertical tab strip not verified|P2|47.0|
|CR-136 `vertical-tabs-minimize-expand-tabs-should-apply-to-each-window-not-every-window`|[original](https://community.brave.app/t/vertical-tabs-minimize-expand-tabs-should-apply-to-each-window-not-every-window/659596)|Not verified for this exact refinement|P2|47.0|
|CR-137 `vivaldi-settings-button-on-the-right-side-windows`|[original](https://forum.vivaldi.net/topic/122006/vivaldi-settings-button-on-the-right-side-windows)|Not verified for this exact refinement|P2|42.0|
|CR-138 `web-challenge-compatibility`|[original](https://github.com/ungoogled-software/ungoogled-chromium/issues/3226)|Not verified for this exact refinement|P1|59.0|
|CR-139 `whole-browser-memory-pressure`|[original](https://github.com/zen-browser/desktop/issues/8932)|Not verified for this exact refinement|P1|59.0|
|CR-140 `workspace-rule-open-in-current-workspace`|[original](https://forum.vivaldi.net/topic/121972/workspace-rule-open-in-current-workspace)|Not verified for this exact refinement|P2|47.0|
|CR-141 `workspace-switching-on-scroll-improvements`|[original](https://forum.vivaldi.net/topic/122048/workspace-switching-on-scroll-improvements)|Not verified for this exact refinement|P2|47.0|
|CR-142 `workspaces`|[original](https://www.reddit.com/r/browsers/comments/1iuwj21/)|Partial: companion saved sessions/groups; isolated native workspaces not verified|P2|47.0|
|CR-143 `workspaces-displayed-as-a-row-above-the-tabs-row`|[original](https://forum.vivaldi.net/topic/122043/workspaces-displayed-as-a-row-above-the-tabs-row)|Not verified for this exact refinement|P2|47.0|

## 1.13.0 implementation delta

The rows above are the pinned November5 baseline/sample, not retroactively rewritten research. Implemented now: current/all-window tab-manager scope, title/recent sorting, bounded pagination, automatic local checkpoints and serialized manual sessions, duplicate-safe restore retries, settings search, original-download resume recheck/advice, signed data transport/persistence and Store/update tooling. Native vertical tab strips/split rendering, E2EE sync, arbitrary extension parity and universal media support remain unclaimed. See IMPLEMENTATION_STATUS.md for tested versus prepared versus blocked.
