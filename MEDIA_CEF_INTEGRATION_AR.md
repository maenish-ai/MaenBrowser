# دمج CEF مع H.264 وAAC — مسار آمن ومتاح للتجربة

**الحالة:** ملفات المصدر جاهزة لبناء حزمة CEF مخصصة، لكن لا توجد حزمة ترميزات مثبتة أو مختبرة مرفقة بهذا ZIP. لا تعلن دعم H.264/AAC قبل نجاح الاختبارات.

## النسخة المطلوبة

- CEF: `152.0.6+g708dc14+chromium-152.0.7977.83`
- Windows x64، وCEF 152 branch 7977، checkout 708dc14
- الإعدادات الموجودة أصلاً في `tools/build-cef-media.ps1`: `proprietary_codecs=true ffmpeg_branding=Chrome`

## ما الموجود في المشروع

1. `.github/workflows/build-cef-media.yml` لبناء الحزمة على Windows runner (قد يتطلب موارد ووقتاً أكثر من المجاني).
2. `tools/build-cef-media.ps1` لبناء نسخة CEF المطابقة وإنشاء ZIP وSHA-256 وmanifest.
3. `tools/verify-cef-media-build.ps1` للتحقق من مخرجات البناء.
4. `tools/fetch-cef.ps1` للتحقق من SHA-256 قبل استخدام الحزمة.
5. `.github/workflows/maenbrowser-ci.yml` يقرأ `MAEN_CEF_ARCHIVE_URL` و`MAEN_CEF_ARCHIVE_SHA256` من GitHub Actions secrets؛ إذا لم توفرا ينتج نسخة stock preview فقط دون إثبات دعم H.264.

## خطوات لاحقة بعد توفر حزمة مبنية وموثوقة

1. شغّل `build-cef-media` على عامل Windows بمواصفات مناسبة، أو ابنِ نفس الإصدار على جهازك.
2. افحص تقرير البناء والنسخة والملفات، واختبر `cefclient` مع فيديو H.264/AAC.
3. ارفع حزمة ZIP على استضافة HTTPS مناسبة مع التحقق من سياسة الحجم والتخزين.
4. اضبط `MAEN_CEF_ARCHIVE_URL` و`MAEN_CEF_ARCHIVE_SHA256` في GitHub Actions secrets.
5. شغّل بناء المتصفح، واختبر MP4 والصوت في **نفس التبويب**، وكذلك تسجيل الدخول ومانع الإعلانات.

**تحذير:** نسخ CEF 89 أو 74 المنشورة على مواقع غير رسمية لا تطابق CEF 152 ولا يجوز إسقاط ملفات DLL منها في المشروع. النجاح في البناء لا يثبت فك الترميز الفعلي.

**ترخيص:** تجربة خاصة لا تعني إذناً بتوزيع ترميزات محمية؛ احصل على توضيح حقوق التوزيع قبل نشر ملف Setup للجمهور.

المراجع: https://github.com/chromiumembedded/cef ، https://cef-builds.spotifycdn.com/index.html ، https://support.google.com/webdesigner/answer/10043691?hl=en
