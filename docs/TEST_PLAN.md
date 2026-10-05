# Windows acceptance and RAM test plan

Release candidates should be tested on 4 GB, 8 GB and 16 GB Windows machines.

Scenarios:
1. Cold start with one blank/new tab.
2. Five ordinary sites, then ten tabs.
3. Gmail + WhatsApp Web + YouTube + two news sites.
4. Download 100 MB and 1 GB files; verify Save As, progress, completion, Open and Show in folder.
5. PDF open/print/download.
6. Camera/microphone permission flow.
7. Incognito/private window and profile separation.
8. Browser restart/session recovery.
9. Unpacked extension load and failure reporting.
10. Installer, upgrade-over-install and full uninstall.

Record total process-tree working set, commit size, CPU, startup time and crashes. Compare the same URL set and idle duration against current stable Chrome, Edge and Firefox. A 'lighter' claim should only be published for scenarios where measured data supports it.
