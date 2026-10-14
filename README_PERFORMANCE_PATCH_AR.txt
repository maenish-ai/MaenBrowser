MaenBrowser - Low-End Performance Patch
======================================

الهدف:
تقليل استهلاك الطاقة والـ CPU والذاكرة على الأجهزة القديمة بدون تعطيل طبقات الحماية أو ميزات المتصفح الأساسية.

التعديلات:
1) Auto resource policy:
   - <= 4 GB RAM أو <= 2 logical processors => UltraLite
   - <= 8 GB RAM أو <= 4 logical processors => Lite
   - لا يتم تعطيل Sandbox أو Site Isolation أو TLS أو GPU/WebGL/Video decode.

2) Cache limits:
   - UltraLite: Disk cache 24 MiB / Media cache 12 MiB
   - Lite: Disk cache 64 MiB / Media cache 32 MiB

3) Background companion work:
   - فحص التبويبات الخاملة: كل 5 دقائق بدلاً من كل دقيقتين.
   - مزامنة شارة الحماية الاحتياطية: كل 15 دقيقة بدلاً من كل 5 دقائق.
   - تغييرات الحماية ما زالت تحدث فورياً عبر protectionRevision؛ هذا لا يضعف الحجب نفسه.
   - عينات الذاكرة/CPU الأصلية أصبحت مخزنة في Session RAM لمدة 10 دقائق بدلاً من طلبها في كل فحص.
   - لا يتم طلب العينة أصلاً إذا لم توجد تبويبات مرشحة للنوم.
   - مهلة فحص سلامة التبويب خُفضت إلى 2.5 ثانية؛ عند الفشل التبويب يبقى مستيقظاً (Fail-safe).

4) New Tab:
   - الأجهزة <=4 threads تحصل على نسخة رسومية أخف: بدون الظلال الثقيلة والخلفية SVG المعقدة والحركات الزخرفية.
   - كل أدوات البحث والوصول السريع والحماية تبقى موجودة.
   - فحص Previous Session يجري مرة واحدة فقط لكل تشغيل للمتصفح، وليس مع كل New Tab.

5) الاختبارات:
   - startup_popup_performance_test.mjs: PASS
   - idle_recheck_test.mjs: PASS
   - protection_sync_regression_test.mjs: PASS
   - companion_api_test.mjs: PASS
   - resource_policy_test.cpp: PASS
   - package_encoding_test.py: PASS
   - navigation_regression_test.py: PASS

ملاحظة:
هذه الحزمة تحتوي فقط الملفات المطلوب استبدالها في المستودع. انسخها بنفس المسارات ثم ارفعها إلى GitHub.
