MaenBrowser 1.9.7 — حزمة تعديلات الإصدار الجديد
================================================

الهدف: إشعار تلقائي بوجود إصدارات GitHub Releases مستقبلية، فتح الروابط
العادية داخل نفس تبويب MaenBrowser، حفظ تدفقات OAuth وتسجيل الدخول،
وتصحيح localhost callback مع HTTPS-only.

هذه حزمة Patch وليست ملف EXE؛ لا نضمن بناء Windows قبل تشغيل GitHub Actions.
لا يوجد أي تغيير مباشر على مستودع GitHub؛ محاولة الكتابة من الموصل رُفضت (403).

تشغيل سريع على Windows مع اتصال إنترنت وGit وPython 3.10+:
  1) فك ضغط هذه الحزمة.
  2) شغّل PREPARE_WINDOWS.cmd في المجلد نفسه.
  3) سيتم نسخ مستودع GitHub، تطبيق التعديلات، وتصدير:
       MaenBrowser-1.9.7-Changed-Files-Upload.zip
       MaenBrowser-1.9.7-Full-Source.zip
       GitHub-Upload-Changed-Files/
  4) ارفع الملفات المعدلة إلى المستودع مع الحفاظ على نفس المسارات.
  5) تأكد من نجاح بناء Windows؛ وبعده ادفع tag باسم v1.9.7 لنشر Release.

إذا كان المشروع موجودًا محلياً:
  py -3 apply_release.py --repo "C:\path\to\MaenBrowser" --output "C:\path\to\MaenBrowser-1.9.7-EXPORT"

ملاحظة: يلزم مستخدمي نسخة 1.9.6 تثبيت النسخة الجديدة مرة واحدة؛
لا يمكن إحداث نظام تحديث في برنامج قديم مثبَّت عن بعد بمجرد إصدار Release.
