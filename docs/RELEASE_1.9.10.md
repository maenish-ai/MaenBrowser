# MaenBrowser 1.9.10 — internal API initialization and origin handling

The 1.9.9 Windows run compiled and passed six native tests but failed on the first
internal API request, before codec playback:
https://github.com/maenish-ai/MaenBrowser/actions/runs/41855846494

CEF has an upstream report of extension requests being assigned Origin:null:
https://github.com/chromiumembedded/cef/issues/4309
This is relevant evidence, not proof of the exact error in that run, which did not
capture network failure details.

Changes:

- Register the local HTTPS factory explicitly on both the global profile context
  and the browser context before loading extension pages. Check registration
  results and stop initialization on failure. Per-context callback registration
  remains for new contexts.
- Authenticate internal requests from CEF's initiator or the actual requesting
  extension frame. A conflicting non-extension initiator is rejected. Never use
  a browser's top-level URL as proof for requests from an untrusted subframe.
- Normalize the Origin header only in mutable requests already authenticated by
  that CEF metadata. CORS continues to name only the exact companion origin.
  An unidentifiable frameless Origin:null request is still denied; this patch does
  not grant access to null origins or bypass an upstream worker bug indiscriminately.
- Before the first diagnostic read, wait a bounded number of times for a read-only
  get response. HTTP denial and invalid responses fail immediately; writes are
  never retried or replayed.
- Collect endpoint-only CDP request/response/failure metadata in
  build/diagnostics/browser-smoke.json, uploaded even on failure. Record no bodies,
  arbitrary URLs, cookies, query tokens or raw headers. Keep existing CEF debug-log
  collection, with bounded source-rejection diagnostics.
- Add origin policy tests and keep their assertions enabled in Windows Release.
  Keep all native API authorization and codec playback gates, including local MP4.

Validation in Linux: portable checks and PowerShell preflight passed. Windows
compilation/playback of this patch remains pending. This is a targeted correction
and diagnostic patch, not a claim that the user's next Actions run has passed.
If a worker has no trustworthy source metadata, it remains fail-closed and the
new diagnostics identify that case for a subsequent verified fix.
