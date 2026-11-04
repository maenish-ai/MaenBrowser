# Current source candidate: 1.9.10

Targeted internal API initialization/origin fix with endpoint-only diagnostics.
The 1.9.9 Windows build compiled and passed six native tests, but failed on its
first internal API call before codec playback. See docs/RELEASE_1.9.10.md.

Portable checks and PowerShell preflight passed in Linux. The updated native
origin test, Windows application compilation and real playback must still pass
on Windows. CEF 154 codec runtime remains pinned and all security/media gates
remain active. No remote edit, Setup or public release was produced here.
