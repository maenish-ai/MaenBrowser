MaenBrowser - GitHub Actions preflight performance fix

سبب الفشل:
أصبح resource_mode.cpp يستخدم cache profile أخف:
UltraLite: 24 MiB disk / 12 MiB media
Lite:      64 MiB disk / 32 MiB media
لكن tools/preflight.ps1 بقي يتحقق من القيم القديمة 96 MiB / 48 MiB، لذلك أوقف GitHub Actions البناء رغم أن اختبارات JavaScript والحزمة نجحت.

الملف المعدل:
tools/preflight.ps1

ما تم تغييره:
تم تحديث regression guard ليتحقق من القيم الفعلية الجديدة فقط، من دون تعطيل أو حذف فحص preflight الأمني.

طريقة الرفع:
انسخ مجلد tools إلى جذر المستودع ووافق على استبدال tools/preflight.ps1، ثم شغّل GitHub Actions مرة أخرى.
