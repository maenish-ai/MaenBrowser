# MaenBrowser 1.9.4

Built on 1.9.3. Keeps on-demand diagnostics and the 1.9.2 automatic media handoff rollback.

## Changes
- Event-triggered download UI refreshes start no more than once per second instead of potentially every 150 ms. Download transfer and action APIs are unchanged.
- Scheduled tab-manager refreshes start no more than once per 500 ms.
- Hidden control pages cancel pending refresh timers. Page-hide also prevents a finishing asynchronous render from scheduling another refresh until the page is restored.
- Event bursts retain one pending update. No idle polling or concurrent renders are introduced by the scheduler.

These changes target control-page overhead; they do not establish a measured reduction in whole-browser CPU and do not add video codec support.

## Validation
Run JavaScript regression tests, portable C++ policy tests, package and encoding validation.
Windows build/runtime remain pending. Test downloads (speed, pause/resume, completion), hidden/restored control pages, and WhatsApp navigation on Windows. Setup must be built with the included GitHub Actions workflow.
