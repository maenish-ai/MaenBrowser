# Performance measurement protocol

**No new browser RAM/CPU/startup benchmark has been run on Windows4GB,8GB or16GB hardware.** Local Linux logic tests, Android compilation and forum complaints are not substitute measurements. No competitor rankings or constant-RAM promise.

The current mode selector uses detected physical memory/logical processors: <=4GB or<=2CPU UltraLite; <=8GB or<=4CPU Lite; >=16GB and>=8CPU Performance; otherwise Balanced. Automatic selection and mode policy are source-tested; workload benefit is unmeasured. Sandbox, Site Isolation and TLS are retained in all modes. A4GB computer may still page heavily on demanding sites.

## Reproducible all-process capture

New `tools/measure-browser-processes.ps1` (PowerShell7.2+,Windows) samples exact executable roots plus observed descendants, including renderers/GPU and any WebView descendants. Capture60seconds minimum after settling:

```powershell
pwsh tools/measure-browser-processes.ps1 -ExecutablePath 'C:\MaenBrowser\MaenBrowser.exe' -OutputPath '.\idle-5tabs.json' -DeclaredTabs 5 -Scenario idle -DurationSeconds 60
```

The tool records OS/build, CPU/logical count, physical memory, app version, sample interval and declared scenario/tab count. It does not open tabs or verify their number; record the workload separately. Private bytes approximate committed private process memory; summed working sets double-count shared pages. CPU is interval process CPU delta normalized over all logical CPUs, not a single-root snapshot. First CPU sample is null; processes that start and exit between snapshots or are inaccessible may be missed; reported totals are an observation, not perfect attribution. No URLs, profile command lines or credentials are captured.

| Hardware tier | OS/CPU/GPU/storage/WebView version | Tabs1/5/10/20 idle | Chat/file/call/video | Startup/switching | Result |
|---|---|---|---|---|---|
|4GB physical|Not provisioned|Not run|Not run|Not run|Unmeasured|
|8GB physical|Not provisioned|Not run|Not run|Not run|Unmeasured|
|16GB physical|Not provisioned|Not run|Not run|Not run|Unmeasured|

Use identical Windows build, power plan, GPU driver, screen resolution, network, browser/engine versions, site content and extension set. Record clean and established profiles, cold/warm startup (external timing), three repeats, median and range; include all browser processes, idleCPU, peak/median private bytes, total system pressure/page faults, battery/thermal observations and dropped frames. Test default and each mode without weakening security. Competing browsers require the same protocol before comparisons. Redact account data.

Discard is conservative: calls/media/downloads/dirty forms/pinned or protected site policies survive, and unknown page inspection preserves tabs. This branch additionally preserves embedded frames/shadow trees/largeDOM or unavailable inspection API; reduced memory savings are acceptable to avoid data loss. Test drafts in cross-origin frames and closedshadow editors on Windows before asserting no-loss behavior. No reload/URL snapshot can recreate every DOM state.

## 1.13.0 tools

See PERFORMANCE_1.13.0.md for selected-root capture, real-Windows1/5/10/20tab synthetic runner and median/range summarizer. No physical tier measurements exist yet. Native Family tables remain lazy while off; checkpoint/manager work is bounded and coalesced.
