import {t,currentLanguage,errorText} from './i18n.js';
import {api,$,message} from './api.js';
import {diagnosticCode,diagnosticReport} from './diagnostics.js';
let state;
let performanceReady=false;
const booleans=['ads','adult','violence','safeSearch','askDownload','httpsOnly'];
const domains=id=>[...new Set($(id).value.split(/\s+/).map(s=>s.trim().toLowerCase().replace(/\.+$/,'')).filter(Boolean))];
async function load(){
  $('retry').disabled=true;$('fields').disabled=true;
  try{
  state=await api();$('language').value=state.language||currentLanguage();
  if(!['exceptions','allowed','blocked'].every(k=>Array.isArray(state[k]))||typeof state.lockedFile!=='boolean')
    throw new Error('PROTECTION_RESPONSE');
  for(const k of booleans)$(k).checked=state[k];
  $('familyEnabled').checked=state.family;$('allowOnly').value=String(state.allowOnly);
  $('downloadDirectory').value=state.downloadDirectory;
  for(const k of ['exceptions','allowed','blocked'])$(k).value=state[k].join('\n');
  $('lists').textContent=t('Local lists: {ads} ad/tracker domains, {adult} adult domains, {violence} violence domains.',{ads:state.adDomains,adult:state.adultDomains,violence:state.violenceDomains});
  $('fields').disabled=state.lockedFile;
  message($('status'),state.lockedFile?new Error('Protected settings could not be read. Family browsing remains restricted. Restore the profile backup or reinstall with a new profile.'):state.listError?new Error('Filter files are missing or empty. Repair the installation.'):'Native settings loaded.');
  $('details').textContent='';
  }catch(error){
    state=undefined;message($('status'),error);
    $('details').textContent=diagnosticCode(error);
  }finally{$('retry').disabled=false;}
}
// Optional extension preferences must never lock the native protection controls.
async function loadPerformance(){
  const controls=['sleepEnabled','sleepMinutes','keepAwake'];
  controls.forEach(k=>$(k).disabled=true);
  try{
    const p=await chrome.storage.local.get({sleepEnabled:true,sleepMinutes:30,keepAwake:[]});
    $('sleepEnabled').checked=p.sleepEnabled!==false;
    $('sleepMinutes').value=String([15,30,60,120].includes(Number(p.sleepMinutes))?Number(p.sleepMinutes):30);
    $('keepAwake').value=Array.isArray(p.keepAwake)?p.keepAwake.filter(x=>typeof x==='string').join('\n'):'';
    performanceReady=true;controls.forEach(k=>$(k).disabled=false);
  }catch(error){message($('performanceStatus'),new Error('Performance preferences could not be loaded. Other settings remain available.'));}
}
$('retry').onclick=()=>{load();if(!performanceReady)loadPerformance();};
$('diagnosticReport').onclick=()=>{$('report').hidden=false;$('report').value=diagnosticReport();$('report').select();};
$('form').onsubmit=async e=>{
  e.preventDefault();$('save').disabled=true;
  try{
    if(!state||state.lockedFile)throw new Error('Could not connect to protection. Please retry or open Settings.');
    if($('newPin').value!==$('confirmPin').value)throw new Error('New PIN confirmation does not match.');
    const settings={family:$('familyEnabled').checked,allowOnly:$('allowOnly').value==='true',downloadDirectory:$('downloadDirectory').value.trim()};
    for(const k of booleans)settings[k]=$(k).checked;
    for(const k of ['exceptions','allowed','blocked'])settings[k]=domains(k);
    const keepAwake=domains('keepAwake');
    if(keepAwake.some(x=>!/^([a-z0-9-]+\.)+[a-z0-9-]+$/.test(x)))throw new Error('Keep-awake entries must be domain names, not full URLs.');
    if(settings.family&&!state.family&&!confirm(t('Enabling Family Protection closes existing web tabs and disables other extensions. Save your work first. Continue?')))return;
    // Presence of `family` is a privileged operation when a PIN exists, even
    // while family mode is off. Do not turn an ordinary ad/download save into
    // a family-mode write. Native enforcement still requires the PIN for every
    // setting while family mode is active and for real family transitions.
    if(settings.family===state.family)delete settings.family;
    const extra={settings,pin:$('pin').value};if($('newPin').value)extra.newPin=$('newPin').value;
    const result=await api('save',extra);
    const wasFamily=state.family;
    state=result;$('pin').value='';$('newPin').value='';$('confirmPin').value='';
    // Enforce a family-mode transition before optional UI preference writes.
    const changed=await chrome.runtime.sendMessage({op:'settingsChanged',family:result.family,wasFamily});
    if(!changed?.ok)throw new Error(changed?.error||'Could not apply settings.');
    if(performanceReady){try{
      await chrome.storage.local.set({sleepEnabled:$('sleepEnabled').checked,sleepMinutes:Number($('sleepMinutes').value),keepAwake});
    }catch(error){message($('performanceStatus'),new Error('Performance preferences could not be saved. Native settings were saved.'));}}
    message($('status'),changed?.error?new Error(t('Native settings saved. {reason}',{reason:errorText(changed.error)})):'Settings saved. Reload open sites for ad-block changes.');
    try{await chrome.action.setBadgeText({text:state.ads?'✓':'—'});}catch{}
  }catch(err){message($('status'),err);}finally{$('save').disabled=false;}
};
load();loadPerformance();

$('language').value=currentLanguage();
$('language').onchange=async()=>{
  const language=$('language').value;$('language').disabled=true;
  try{
    await api('setLanguage',{language});
    await chrome.storage.local.set({language});
    message($('languageStatus'),'Language saved. Browser menus change after closing all windows and reopening MaenBrowser.');
  }catch(error){$('language').value=currentLanguage();message($('languageStatus'),error);}
  finally{$('language').disabled=false;}
};
