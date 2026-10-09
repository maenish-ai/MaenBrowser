// Local, bounded diagnostics. Never record URLs, domains, requests, PINs or raw errors.
import {currentLanguage} from './i18n.js';
export const errorCatalogue={
  E101:['The extension worker did not recognize the command. Close all browser windows and install the matching version.','عامل الإضافة لا يعرف الأمر. أغلق جميع نوافذ المتصفح وثبّت الإصدار المطابق.'],
  E102:['The local protection service did not respond. Retry the connection.','لم تستجب خدمة الحماية المحلية. أعد محاولة الاتصال.'],
  E103:['The local service rejected this panel origin. Repair the installation.','رفضت الخدمة مصدر لوحة التحكم. أصلح تثبيت المتصفح.'],
  E104:['The local service returned an HTTP error. See the status in the report.','أعادت الخدمة خطأ HTTP. راجع رقم الحالة في التقرير.'],
  E105:['The local service returned incomplete or invalid data. Repair the installation.','أعادت الخدمة بيانات ناقصة أو غير صالحة. أصلح التثبيت.'],
  E106:['The native browser does not recognize this operation. Panel and browser may not match.','المتصفح لا يعرف هذه العملية. قد لا يتطابق إصدار اللوحة مع المتصفح.'],
  E107:['The settings contain an invalid value. Check domains and the download folder.','تحتوي الإعدادات قيمة غير صالحة. راجع أسماء المواقع ومجلد التنزيل.'],
  E108:['The parent PIN is missing or incorrect.','رمز الوالدين مفقود أو غير صحيح.'],
  E109:['Protected settings cannot be read. Family restrictions remain in place.','تعذرت قراءة الإعدادات المحمية. تبقى قيود الأسرة مفعّلة.'],
  E110:['Settings could not be saved. Check write access and free disk space.','تعذر حفظ الإعدادات. راجع صلاحية الكتابة والمساحة المتاحة.'],
  E111:['Wait before retrying the parent PIN.','انتظر قبل إعادة محاولة رمز الوالدين.'],
  E112:['Protection filter files are missing or empty. Repair the installation.','ملفات قوائم الحماية مفقودة أو فارغة. أصلح التثبيت.'],
  E199:['An unclassified protection error occurred. Copy the diagnostic report.','حدث خطأ حماية غير مصنف. انسخ تقرير التشخيص.']
};
const nativeMessages={
  'Unknown action':'E101','Unknown operation':'E106',
  'Incorrect parent PIN.':'E108','Set a parent PIN before enabling family mode.':'E108',
  'Please wait before trying your PIN again.':'E111',
  'Protected settings cannot be read. Restore your profile backup or reinstall with a new profile.':'E109',
  'Protected settings could not be read. Family browsing remains restricted. Restore the profile backup or reinstall with a new profile.':'E109',
  'Filter files are missing or empty. Repair the installation.':'E112',
  'Invalid settings. Use domain names only and an absolute download folder.':'E107',
  'Missing settings':'E107','Use a parent PIN or passphrase of 6 to 128 characters.':'E107',
  'Could not save protected settings. No changes were applied.':'E110','Could not save language.':'E110',
  'Family category lists are missing. Repair installation or use allowed-sites-only mode.':'E112'
};
export function diagnosticCode(error){
  const c=error?.code||error?.message||String(error);
  if(errorCatalogue[c])return c;
  if(c==='PROTECTION_WORKER')return 'E101';
  if(c==='PROTECTION_NETWORK')return 'E102';
  if(c==='PROTECTION_RESPONSE')return 'E105';
  if(c==='PROTECTION_HTTP_403')return 'E103';
  if(/^PROTECTION_HTTP_\d+$/.test(c))return 'E104';
  return nativeMessages[error?.message||String(error)]||'E199';
}
export function describeError(error){const code=diagnosticCode(error);return code+' — '+errorCatalogue[code][currentLanguage()==='ar'?1:0];}
const events=[];
let nativeVersion=null;
let protection=null;
export function recordDiagnostic(operation,error,state){
  if(state?.version)nativeVersion=String(state.version).slice(0,24);
  if(state)protection={ads:state.ads===true,family:state.family===true,locked:state.lockedFile===true,filterError:!!state.listError,adDomains:Number(state.adDomains)||0,blockedRequests:/^\d+$/.test(String(state.blockedRequests))?String(state.blockedRequests):null};
  const code=error?diagnosticCode(error):'OK';
  const stage=code==='E101'?'worker':code==='E102'?'transport':['E103','E104'].includes(code)?'http':code==='E105'?'response':error?'native-validation':'native-http';
  const event={time:new Date().toISOString(),stage,operation:['get','save','setLanguage'].includes(operation)?operation:'unsupported',code};
  const http=String(error?.code||'').match(/^PROTECTION_HTTP_(\d+)$/);if(http)event.httpStatus=Number(http[1]);
  events.push(event);if(events.length>20)events.shift();
}
export function diagnosticReport(){return JSON.stringify({schema:1,panel:chrome.runtime.getManifest().version,native:nativeVersion,transport:'direct-local-endpoint',protection,events},null,2);}
