MaenBrowser - Installer update patch

الملف المطلوب استبداله:
installer/MaenBrowser.nsi

التغييرات:
1) عند تشغيل Setup تظهر نافذة اختيار اللغة: English / العربية.
2) لغة Setup بالكامل تتبع الاختيار عبر NSIS MUI.
3) الاختيار نفسه يُكتب إلى:
   %LOCALAPPDATA%\MaenBrowser\ui-language.txt
   وبالتالي يبدأ MaenBrowser باللغة المختارة من أول تشغيل بعد التثبيت.
4) إذا كان MaenBrowser مفتوحاً، يحاول Setup إغلاق نوافذه بالطريقة العادية
   (CloseMainWindow / WM_CLOSE) وينتظر حتى 10 ثوانٍ ثم يكمل التحديث تلقائياً.
5) لا يوجد taskkill /F ولا قتل إجباري للعمليات؛ إذا لم يغلق المتصفح، يعرض Setup
   Retry/Cancel لحماية ملف المستخدم والجلسة من التلف.
6) التحديث يحافظ على %LOCALAPPDATA%\MaenBrowser ولا يحذف ملف المستخدم أو بياناته.

ملاحظة اختبار:
هذا تعديل مصدر NSIS. يجب أن يمر عبر GitHub Actions لبناء Setup فعلي، ثم اختبار
الترقية على Windows مع وجود نسخة قديمة مفتوحة.
