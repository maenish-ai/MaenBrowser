# Windows release acceptance — 1.9.6

Status of every Windows row below: **NOT RUN in the source-preparation environment**.
Record hardware, OS build, actual CEF version, tester, date and result before distribution.

| Area | Required check | Pass condition |
|---|---|---|
| Build | Run release Actions with verified media archive | Compile, CTest, browser_smoke and NSIS all succeed |
| Install | Clean install and upgrade from 1.9.5 | Starts, extension loaded, bookmarks/profile/PIN preserved |
| Uninstall | Uninstall and reinstall | Profile retained; no unrelated files removed |
| Blocking | Popup OFF/ON, exceptions, restart | Actual request interception follows chosen persistent state |
| Conflicts | Open options, toggle ads in popup, save unrelated option | Stale save rejected, no hidden ad-state reversal |
| Recovery | Corrupt copy of profile controls.dat in disposable Windows user | Family remains locked; backup PIN required, no unlocked fallback |
| Filters | Disconnect network/malformed response/write failure/update success | Old rules preserved on failure; valid new list loads after restart |
| Family | Enable category/allow-only policies, attempt external extension | PIN required; category lists load; extension disabling explained |
| Resources | Same sites on 2 GB and 4 GB Windows | Record full process-tree private memory/working set/CPU/startup/tab latency |
| Tab safety | Forms, edited-to-empty input, music, camera, screen capture, download | Work kept; safe stale tabs only released, user exemptions respected |
| Sessions | Restore 20 saved sites | One real initial load; background URLs load when selected |
| Codecs | Automated fixture matrix | H.264/AAC, VP8/Vorbis, VP9/Opus, AV1/Opus, MP3/FLAC/WAV/Vorbis/AAC decode |
| Output | Real speakers/headphones, seeking, fullscreen | Audible undistorted output and controls work |
| YouTube | Logged-out and signed-in playback | Controls, seek, subtitles and supported resolution |
| WhatsApp Web | Sign-in, attachments, received MP4, voice | Files upload/download, audio/video actually play |
| Gmail / Outlook Web | Sign-in, compose, attachments | Editing and file transfer work |
| Docs / Microsoft 365 | Document edit, save, export | No loss of changes; save/export confirmed |
| Meet / Teams / Zoom Web | Join call, camera/mic, share screen if service supports it | Explicit permissions; revoke works; no discard during call |
| Windows | HTTP/HTTPS default choice, PDF, printing, file chooser | User-selected associations and normal dialogs work |
| External apps | Mail/tel/Zoom/Teams links | User gesture and confirmation required; Cancel launches nothing |
| Origin security | Ordinary site and another extension call local API | No read/write allowed; worker/popup of bundled extension allowed |
| TLS | Invalid certificate test | No silent certificate bypass |
| Permissions | Deny, allow, revoke camera/mic/location/notifications | Correct prompts and revocation; no silent grants |
| Extensions | User's representative extension set | Record supported/unsupported cases; no universal claim |
| DRM / HEVC | Record OS/GPU/service/CDM requirements | Separate capability decision, never inferred from H.264 success |

## Performance procedure
Use identical Windows power settings, sites, network conditions, tab counts and
warm/cold-cache conditions for 1.9.5 and 1.9.6. Run three repeats: start/new tab,
one light site, five sites, ten sites, video, then tab close/recovery. Use
`tools/measure-windows.ps1 -Seconds 120 -OutputPath before.csv` and again after.
PrivateMB and working set cover the process tree; working set can double-count
shared pages. Report median/range, UI responsiveness and disk pressure; do not
claim a saving from the main process alone. 2 GB cannot guarantee heavy sites or
high-resolution media. Select realistic workloads and retain adequate OS headroom.

## Release blockers
A compile, native API, request blocking, persistent-state, sandbox, permission,
profile preservation or required fixture playback failure must be resolved before
release. External authenticated services require user/device testing. Distribution
signing and any DRM/codec agreements must be supplied by the publisher. This source
package does not enable unsigned automatic application updates.
