# 1.9.2 — rollback of automatic media handoff

Preserves all 1.9.1 diagnostics, idle scan serialization and 1.9.0 FIX1.
Disables the shared routing policy used by both main-frame navigation and
resource interception. WhatsApp and direct media addresses now load normally
in CEF, without replacing the server response with a local animated placeholder
and without starting a second engine. Family/domain filtering remains active.

This is a rollback, not proof of the Windows handoff failure's underlying cause.
It removes the code path responsible for the pictured waiting screen.
Stock CEF codec limitations remain: this release does not guarantee WhatsApp
video/audio or every website player. Existing WebView2 sign-in data is retained,
but CEF has a separate session so WhatsApp may ask for QR sign-in again.

Tests: JavaScript suite, portable C++ routing/domain/navigation tests, package
localization and encoding validation. Full Windows build/runtime pending.
After Actions builds Setup: verify 1.9.2 in About, open WhatsApp, check that the
MaenBrowser waiting screen no longer appears, test login and media separately,
then check protection and downloads. Use Inspect page video for failing players.

تم إلغاء اعتراض صفحات واتساب وروابط الوسائط مع الاحتفاظ بتعديلات 1.9.1.
قد تحتاج لتسجيل دخول واتساب مجددًا لاختلاف جلسة المحرك. بيانات الجلسة السابقة
لا تُحذف. لا تدّعي هذه النسخة إصلاح جميع ترميزات الفيديو. ارفع المحتويات كاملة
بما فيها .github، ثم ابنِ Setup واختبره على ويندوز.
