# MaenBrowser 1.9.1 — on-demand media diagnostics

Base: 1.9.0 FIX1 source, preserving the CEF BindOnce build correction and removal
of automatic media reloads. This release does not restore automatic engine switching.

Changes:
- Shield > Inspect page video captures a read-only snapshot of accessible CEF
  frames, with media errors, ready/network states and codec capability hints.
- Reports contain no URLs, file names, cookies or raw website error messages.
- At most 20 media elements per frame are reported. No media observers/timers
  or background page probes are installed. Reports are selectable in the panel.
- Reported HTML media errors feed the existing bounded local error journal.
- Idle tab scans cannot overlap while an earlier scan is running.
- Version updated to 1.9.1; Arabic and English controls included.

Limits: this does not add proprietary codecs or repair every failing site.
Source/format errors do not prove a codec defect. A mediaKeys object indicates
an encrypted-media context, not a proven DRM fault. WebView2 and inaccessible
frames are not inspected. Empty results do not establish successful playback.
The installed-device symptom and Windows build have not been verified here.

Local checks: 15 JavaScript tests, package assets/localization/encoding checks;
portable C++ domain, media policy and internal navigation tests.

Windows acceptance:
1. Build Setup in Actions; install with all MaenBrowser windows closed.
2. Open About from the menu; check version 1.9.1 and export a snapshot.
3. Scroll an ordinary page containing a failing video: no automatic reload.
4. Open shield > Inspect page video, copy the report, then test a working video.
5. Check WhatsApp media separately, protection switches and download controls.
6. Compare idle CPU/RAM on the same device and pages; no measured speedup claimed.

فك الضغط وارفع المحتويات إلى جذر المستودع، بما فيها .github. هذه حزمة مصدر
وليست Setup. عند فشل فيديو: افتح الحماية > فحص فيديو الصفحة، ثم انسخ التقرير.
لا يلزم فتح Console. التقرير لا يشمل صفحات WebView2 ولا يضمن إصلاح الترميز.
