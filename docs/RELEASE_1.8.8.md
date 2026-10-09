# 1.8.8 — protection connection and numbered diagnostics

The user screenshot shows panel 1.8.7 reporting `PROTECTION_UNKNOWN · Unknown action`, despite the successful Windows run https://github.com/maenish-ai/MaenBrowser/actions/runs/37968620382 . In source, `Unknown action` is the extension worker's command-dispatch fallback; the native C++ API instead says `Unknown operation`. This points to an unrecognized worker request, plausibly stale worker code after upgrade. The exact installed worker bytes were not available to confirm the cache hypothesis.

## Fix

- Privileged extension panels now use the origin-checked local HTTPS API directly for state, settings and language. The old worker is no longer in that control path. Each write is sent once, with no automatic replay.
- The manifest uses a new background entrypoint URL to replace the prior worker registration during extension update. Existing profiles, settings, parent PIN and saved sessions are preserved.
- Numbered bilingual error explanations appear at the top of the popup. A selectable diagnostic report is available in both popup and settings. See ERROR_CODES.md. Logs are local to the open page and bounded, without browsing data or credentials.
- All existing native authorization, family PIN checks, origin checks and filter enforcement remain intact. No fake ON state or swallowed API error is used.

## Validation

Local tests simulate the reported old-worker `Unknown action` response and verify that native reads/writes are independent of it, writes are not replayed, HTTP/PIN/response failures are classified, and reports omit private data. A UI regression verifies visible red E101, retry and report access. Existing Windows smoke tests still exercise the real popup, checkbox saves, native ad-request interception and untrusted-origin rejection.

These local tests passed; the new Windows build/integration result is pending upload. Success of 1.8.7 does not certify 1.8.8. Domain filtering does not remove all first-party or in-stream advertisements.

## تجربة المستخدم

أغلق جميع نوافذ المتصفح ثم ثبّت Setup الجديد بعد نجاح Actions. افتح الحماية وتحقق من رقم اللوحة والمتصفح 1.8.8. إذا عاد الخطأ يظهر رمز E مع شرح؛ افتح «تقرير التشخيص» وانسخ محتواه. التقرير ليس إرسالًا تلقائيًا، ولا يتضمن المواقع أو رمز الوالدين. لا تحذف ملف المستخدم لمحاولة الإصلاح.
