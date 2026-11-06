# Media compatibility matrix

Scope: pinned CEF154 codec-enabled distribution; baseline [Windows CI](https://github.com/maenish-ai/MaenBrowser/actions/runs/42094561247). MP4 is a container; file extension alone does not identify codec, profile, encryption, MIME, network or authentication failures. No universal codec promise.

| Format/API | Current evidence | Acceptance remaining |
|---|---|---|
| MP4 / H.264 / AAC | Codec runtime locked; actual baseline native H.264 Main/High fixtures and audio/video checks | Additional levels, fragmented MP4, VFR, malformed media, real authenticated platforms |
| WebM / VP8 / VP9 / Opus | Runtime support and fixture coverage vary; baseline Opus decode confirmed | Per-codec actual decode on candidate and physical hardware |
| AV1 | Engine capability conditional on build/device; no new physical-device evidence | Software/hardware decode, low-power fallback and resource measurement |
| MP3 / FLAC / WAV / Ogg | Engine/fixture definitions, not universal acceptance | Every codec/container pair, seeking, long playback, Bluetooth switching |
| PNG / JPEG / WebP / GIF / SVG / AVIF | Baseline native decode assertions passed; BMP also tested | Corrupt input, huge images, color management, candidate engine |
| JPEG XL | Community request, not guaranteed enabled engine support | Investigate supported upstream path; no arbitrary decoder DLL injection |
| HEVC | Platform/build/licensing dependent | Not promised; rights plus genuine decoder support and native tests |
| DRM / protected streams | CDM/platform/service contracts required | No bundled unauthorized Widevine or DRM bypass; service-specific licensing |
| MSE / adaptive streaming / subtitles / tracks | Chromium features, website/build dependent | Real DASH/HLS path supported by each site; track switching and captions |
| WebRTC / WebAudio / WebCodecs | Engine APIs, not proof of complete call experience | Real two-party calls, device selection, permission lifecycle, echo, screen-share/audio |
| PiP / fullscreen | Runtime/browser UI integration | Multiple monitors, gestures, keyboard, platform restrictions |
| Hardware decode / software fallback | Depends GPU/driver/build | Record GPU/driver, decoder in diagnostics, 1080p/4K CPU/dropframes; avoid global GPU disable |
| Android media | Installed WebView + Android codecs/hardware | Real devices/OS/WebView versions, calls/permissions/uploads; desktop fixtures do not certify Android |

## Engine/session boundaries

CEF is the normal engine. `kEmbeddedMediaRoutingEnabled=false`; WebView2 is a legacy opt-in/lazy compiled path. Code presence alone does not imply both engines consume memory on every page. If fallback is invoked, expect a separate request context, cookies and permissions: no copied credentials, no hidden camera/mic grants, no replay of authenticated POST downloads. Measure both process trees before considering removing or reviving that path.

For a playback failure capture non-sensitive diagnostics: application/CEF/WebView version, MIME, canPlayType/MediaCapabilities result, HTMLMediaElement.error, network status and renderer/GPU crash outcome. Strip signed URLs, tokens and cookies from exported logs. Verify a local known-good fixture, then an HTTPS fixture, then actual site authorization. Installing a standalone codec pack does not add decoders to Chromium's own FFmpeg build.

Current web Adobe/Office applications are web-compatibility targets, not Flash/NPAPI plugins or native desktop programs. No legacy unsafe plugin reactivation. [Chromium media](https://www.chromium.org/audio-video/), [Android WebView compatibility](https://chromium.googlesource.com/chromium/src/+/HEAD/android_webview/docs/web-platform-compatibility.md), [engine plan](ENGINE_UPGRADE_PLAN.md).

## 1.13.0 scope

No new decoder DLLs were mixed in. Active CEF154 codec lock stays unchanged; earlier native decode evidence applies to that baseline, not new branch acceptance. New engine pipeline runs real codec/download/protection integration before considering lock promotion. Android codecs remain installed-WebView/OS dependent.
