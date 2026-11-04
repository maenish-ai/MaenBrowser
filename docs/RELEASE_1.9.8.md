# MaenBrowser 1.9.8

Chat and creative application tabs are protected from automatic idle discard on
WhatsApp, Discord, Slack, Telegram, Google Chat, Gmail, Outlook, Adobe, Figma and
Photopea domains. Generic forum chats exposing editable fields, textbox/log/chat
roles, drafts, active media or camera/microphone grants are also kept awake.
Custom domains can be added to the existing keep-awake list. This does not detect
every site's private WebSocket state; add your forum domain for that guarantee.

Idle inspection avoids copying all controls into JavaScript arrays and keeps
complex pages awake once the control/media limit is exceeded. It installs no
persistent page script or network interception. No speedup percentage is claimed.
Protection badges refresh after settings changes. Existing sandbox, site isolation,
TLS, family protection and GPU safety decisions remain in place.

Release C++ assertions remain enabled. Tagged releases require matching VERSION
and a media-enabled CEF archive; the workflow builds a draft GitHub release only
after native tests and real browser/codec integration pass. Setup SHA-256 files
are included. Stock-CEF previews remain clearly labeled and cannot create a
release through this path.

CEF build prerequisite checks now fail early without Windows x64, VS2022 C++ or
300 GB free build storage. Archive URLs require HTTPS and the media manifest hash
must agree with the archive hash. Codec support requires the actual custom CEF
build, not this source package alone.

Validation in the preparation environment: portable JavaScript/Python/C++ checks
passed. Windows compilation, proprietary codec binary build, runtime media tests,
WhatsApp login/calls, Adobe web applications and device performance are pending.
No Windows Setup or codec DLLs have been produced in the Linux environment.
H.264/AAC, VP8/Vorbis, VP9/Opus, AV1/Opus, MP3, FLAC, WAV and Ogg have Windows
playback fixtures. HEVC, DRM/Widevine, uncommon codecs and all Internet services
are not guaranteed. Adobe Flash is obsolete and is not enabled.
