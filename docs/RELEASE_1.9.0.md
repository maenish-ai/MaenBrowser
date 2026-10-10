# MaenBrowser 1.9.0 — navigation fixes

Base: maenish-ai/MaenBrowser commit 678e45000e4d6c645b0da0068a208b8b196712c1 (1.8.9).

## Changes
- Removed the page-console/media-probe bridge that called LoadURL on the current page after an unsupported media error. A failing video or page-generated console marker can no longer trigger that browser-owned navigation. Ordinary pages retain their current document and scroll position; no scroll-reset/restore script is injected.
- Removed the repeated full-document MutationObserver media scans associated with that bridge. Ordinary pages stay in CEF; WhatsApp and supported direct HTTPS media retain their existing WebView2 routing. Unsupported media on other websites may still fail to play.
- Consume a pending embedded-media navigation before opening its surface, preventing duplicate load-end notifications from reopening/re-navigating it.
- Intercept Chromium IDC_ABOUT and open the existing local companion about.html Technical Details page. Explicit chrome://settings/help, chrome://help, chrome://about, chrome://version and maen://about / maen://technical-details aliases route there too. Chromium's built-in menu label is not renamed.
- Preserve all 1.8.9 diagnostics, security, profiles, settings and idle-tab protections. No new speculative error codes or automatic engine retry.

## Validation performed on Linux
- 14 JavaScript unit scripts passed.
- 3 compiled C++ tests passed: domain rules, media routing and internal URL alias boundaries.
- Native navigation source regression guards passed (these do not execute CEF callbacks).
- Package assets, JavaScript syntax, translation coverage, filter hashes and simulated Windows encoding validation passed.
- git diff --check passed.

## Not validated
No Windows application build or live CEF/WebView2 UI test was possible here. browser_smoke.mjs requires a configured browser test environment; an initial invocation without its required argument failed before executing browser checks. This archive is source, not an installer, and is not a claim of runtime certification. Site-owned refresh, Chromium memory-pressure discard and user-enabled idle discard are not disabled.

## الرفع والتجربة
1. فك الضغط وارفع محتويات مجلد MaenBrowser-1.9.0 إلى جذر المستودع، بما فيها .github. لا ترفع ZIP وحده.
2. شغّل Build MaenBrowser Windows Setup من Actions ثم نزّل ملف التثبيت الناتج.
3. افتح قائمة النقاط الثلاث ثم About Chromium: يجب أن تظهر معلومات MaenBrowser المحلية. جرّب chrome://settings/help وmaen://about أيضًا.
4. افتح صفحة طويلة، انزل لمنتصفها وشغّل فيديو غير مدعوم: يجب ألا يعيد المتصفح تحميل الصفحة بسبب خطأ الفيديو. جرّب أيضًا console.warn('MAEN_MEDIA_UNSUPPORTED') في أدوات المطور: يجب ألا يسبب تنقلًا.
5. جرّب WhatsApp وملف MP4 مباشر، التبديل بين التبويبات، الرجوع والتقدم وإعادة التحميل اليدوية والتنزيلات، ثم أعد اختبار About مع حماية الأسرة مفعلة.
6. إذا استمر الرجوع للأعلى، سجّل الموقع والخطوات وهل يحدث في تبويب نشط أو عند إعادة فتح تبويب خامل، لتحديد سبب آخر.
