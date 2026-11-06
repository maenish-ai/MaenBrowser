# Performance evidence and executable protocol

No physical4/8/16GB Windows benchmark was run here. No fixed RAM figure, startup speedup percentage or competitor ranking is claimed. Smaller compressed installer size is not equivalent to lower working RAM, CPU or latency. CEF's codec runtime archive is173,759,232bytes at the active pin; that is not the final installer or installed footprint.

Concrete low-overhead changes: tab-manager rendering starts at100rows with explicit bounded pagination/search; automatic checkpoints are event-driven and coalesced into a one-shot minute alarm, hash/identity deduplicated, capped3×512KiB; manual session collection is bounded1MiB; signed data checks run daily only and do not poll sites or install another engine. Family classification tables remain lazy while off. Normal browsing remains CEF with WebView2 routing disabled. Hardware/GPU/TLS/site isolation are not disabled to inflate benchmark results.

## Real Windows synthetic suite

With Node22+ and PowerShell7.2+, close unrelated work if comparing total-system pressure, then:

```powershell
node tools/benchmark-workload.mjs C:\MaenBrowser\MaenBrowser.exe C:\measurements\auto auto 60
python tools/summarize-performance.py C:\measurements\auto\*-tabs.json --output C:\measurements\auto-summary.json
```

The runner uses its own temporary profile, loopback-only fixture/CDP endpoints, verifies the browser PID, creates1/5/10/20fixture tabs, closes only its initial test pages, verifies tab count, settles10seconds, captures the selected root and descendants, and cleans up only its own process/profile. Record external GPU/driver, display, power plan and storage details. Repeat with ultra/lite/balanced/performance on each physical tier and at least3runs per workload.

Fixture is static local text with an unsaved textarea, **not WhatsApp, a real call or video playback**. `debugEndpointReadyMs` is technical endpoint readiness, not visual first-paint startup latency. Per-case sidecars record before/after target counts and exact browser version. The capture tool's standalone declared tab count remains user-supplied; only the workload sidecar provides independent CDP count evidence.

Standalone collector supports `-RootProcessId` to avoid attributing other profiles to this test. Defaults still collect all matching installation roots and descendants. Summed working sets double-count shared pages; private commit is not resident RAM; newly exited/inaccessible processes may be missed. First CPU interval is null. Summarizer preserves missing data and computes range/median only from real supplied samples. Parsing/unit checks are not execution of these Windows tools.

| Device |1/5/10/20tabs|Idle CPU/commit|Real chat/call/video|Status|
|---|---|---|---|---|
|4GB Windows|Not run|Not measured|Not run|Hardware required|
|8GB Windows|Not run|Not measured|Not run|Hardware required|
|16GB Windows|Not run|Not measured|Not run|Hardware required|

Use [the broader protocol](PERFORMANCE_BENCHMARKS.md) for real authorized sites, battery/heat, tab switching, startup timing, mixed workloads and security/no-loss acceptance. A local logic/UI test does not close this matrix.
