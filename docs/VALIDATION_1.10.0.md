# Validation record: 1.10.0

Base browser: 8fc1aa71df0876cf86e75b976dc92da65db73787, the user's successful
1.9.10. Base Agent: 10dce4b (main as inspected for this task).

Completed locally in Linux:

- Portable JS/Python/C++ checks, with assertions active in native policy tests.
- UTF-8 package validation under simulated Windows cp1252 defaults.
- Pinned domain snapshot hashes/counts, packaged shim hashes/licenses and shim
  callback tests; family-first selection, exact host/path/type selection, private
  native API CORS precedence over public static-script CORS.
- Agent profile overlap/relative-path rejection; protected-file change/delete/
  error handling; startup policy snapshot and mutation denial integration checks.
- PowerShell preflight and script syntax, workflow YAML parsing, and JS syntax.
- The Agent's 63 targeted tests passed, including mocked local CDP adapter,
  explicit browser choice, no silent fallback, owned-process cleanup, existing
  extension/transport/Facebook-health/desktop-navigation regression tests.
- A narrow native domain-lookup microbenchmark is described separately in
  PERFORMANCE_1.10.0.md. It does not measure browser/device performance.

Not completed here:

- Windows compilation/linking and sandboxed launch of 1.10.0.
- New real browser tests: singleton same-tab relaunch, anonymous-cross-origin
  compatibility shim, H.264 Main/High, Opus/Ogg, optional legacy probes, images.
- Windows CDP connection and profile ownership against actual YourAssistantAgent;
  Facebook login/2FA, groups/actions or posting; Adobe editing/import/export;
  arbitrary extension stores/native messaging; user-device RAM/CPU/chat latency.
- Public release, remote repository mutation or production account actions.

The already-working codec archive is unchanged, but that alone does not establish
these new acceptance results. CI keeps required playback/security tests; optional
legacy codec results are recorded as unavailable when decoding fails. The source
package is not a built Setup or a universal compatibility claim.
