# MaenBrowser 1.8.6 — settings recovery and media startup handling

## Changes

- Native settings no longer depend on optional extension performance preferences loading successfully. Malformed older preferences are normalized; unavailable preferences remain disabled independently. A failed native connection has a retry button and a diagnostic code outside the locked fieldset. Corrupt protected settings still remain locked.
- Worker replies have a bounded wait. Optional language persistence no longer delays the protection response. Writes are never retried automatically.
- Family enforcement runs before optional preference writes. An unavailable badge no longer makes a successful save appear to fail.
- Media startup checks synchronous controller creation, CoreWebView2 acquisition and navigation results, and handles renderer/browser process exits. An initialization watchdog and an ordinary loading/error view replace indefinite empty startup surfaces. The controller is shown after a successful navigation. No engine-conversion/setup messages are shown during normal loading. TLS errors are not bypassed.
- The on-demand media engine now follows Lite mode's preload and cache restrictions, while retaining GPU decoding and security defaults. Ordinary pages do not launch this engine.
- A regression test covers unavailable/malformed preferences, protected profile locking and retry. Windows integration additionally clicks and saves the actual ad-block checkbox in both directions.

## Validation and limits

Local JavaScript and source/package checks are possible on Linux. Full Windows CEF/WebView2 compilation and playback are not available in this environment; the added Windows integration tests run after upload. A successful build is not proof that every website's video works.

These changes handle specific settings and blank-startup failure paths; they do not establish the cause of every reported white page. Embedded website players still use CEF's available codecs. The auxiliary engine uses its own session; signed media URLs may require cookies or a referrer from the original website. DRM and unsupported codecs are not fixed by exposing controls. No arbitrary cookie copying, certificate bypass or background transcoding was added.

This release must be tested on the affected Windows device before distribution. Check settings save/reopen, ads on/off on a known fixture, WhatsApp playback/calls, a benign MP4 and embedded video, seeking/volume/fullscreen, tab switching, downloads and idle CPU/RAM/disk. The supplied performance recorder can measure changes; no measured speed or memory improvement is claimed yet. This is a Windows source package, not an Android/Google Play release.

## الرفع والتجربة

فك ضغط الملف وانسخ محتوياته إلى جذر المستودع مع استبدال الملفات القديمة، بما فيها مجلد `.github`. انتظر نجاح Actions ثم نزّل Setup الجديد. أغلق كل نوافذ المتصفح قبل التثبيت.

تتضمن النسخة معالجة لتعطّل الإعدادات عند فشل التفضيلات، وإعادة محاولة الاتصال، ومعالجة حالات فشل بدء الفيديو بدل ترك شاشة بيضاء، وتطبيق قيود الأجهزة الضعيفة على محرك الوسائط أيضًا. لم يُختبر تشغيل الفيديو على جهاز المستخدم بعد، ولا يصح اعتبار جميع المواقع أو الأزرار مضمونة لمجرد نجاح البناء. إذا استمر تعطل الإعدادات، يظهر رمز التشخيص أعلى الصفحة بجوار إعادة المحاولة.
