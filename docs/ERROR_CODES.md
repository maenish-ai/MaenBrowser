# Protection diagnostic codes / رموز تشخيص الحماية

Codes are stable identifiers, not HTTP status numbers. The panel and settings show the localized explanation; Diagnostic report selects a copyable JSON report. Reports are held only in the open page's memory (last 20 API results), and reset when it closes. No automatic upload or persistent log is used. No URLs, exception lists, PINs or raw request bodies are recorded.

| Code | Meaning / المعنى | Action / الإجراء |
|---|---|---|
| E101 | Worker command mismatch / عامل الإضافة لا يعرف الأمر | Close all browser windows; install matching release. Native protection pages in 1.8.8 bypass this worker. |
| E102 | Local service transport failed / تعذر الاتصال بالخدمة المحلية | Retry; inspect panel/native versions; repair installation if persistent. |
| E103 | Local API denied origin (403) / رُفض مصدر الطلب | Repair the bundled extension/runtime pair; do not relax origin validation. |
| E104 | Other HTTP failure / خطأ HTTP آخر | Include report's httpStatus. |
| E105 | Invalid/incomplete response / بيانات استجابة غير صالحة | Check panel/native version alignment. |
| E106 | Native operation unsupported / المتصفح لا يعرف العملية | Install the matching native browser version. |
| E107 | Invalid settings / قيم إعدادات غير صالحة | Check domain names, absolute download directory and PIN length. |
| E108 | Parent PIN missing or wrong / رمز الوالدين مفقود أو خاطئ | Enter the correct PIN; protection is not bypassed. |
| E109 | Protected settings unreadable / إعدادات محمية غير قابلة للقراءة | Restore a known profile backup; family restrictions remain active. |
| E110 | Persistence failure / تعذر الحفظ | Check filesystem permissions and free disk space. |
| E111 | PIN retry rate limit / مهلة إعادة محاولة الرمز | Wait before retrying. |
| E112 | Filter files absent / قوائم الحماية مفقودة | Repair installation. |
| E199 | Unclassified error / خطأ غير مصنف | Send the diagnostic report; do not interpret this as a specific root cause. |

Report fields: schema, panel version, last known native version, transport, protection state (ads/family/locked/filter error/domain count/blocked-request count), and bounded events (UTC time, stage, operation, code, optional HTTP status). Unknown native version remains null; it is never assumed equal to the panel.

Developer tracing: `assets/companion/diagnostics.js` owns classification; `api.js` records direct local API outcomes; native API and filter enforcement live in `src/protection/protection.cpp`. An E101 response from an older worker cannot determine whether native filtering itself is running. Only a successful state read and a real blocked-request test provide that evidence.
