import {downloadAdFilters} from './filter-update.js';
import {changedSettings} from './settings-patch.js';
import {t,currentLanguage,errorText} from './i18n.js';
import {api,$,message} from './api.js';
import {diagnosticCode,diagnosticReport} from './diagnostics.js';
let state;
let performanceReady=false;
let performanceBaseline=null;
const booleans=['ads','adult','violence','safeSearch','askDownload','httpsOnly'];
const domains=id=>[...new Set($(id).value.split(/\s+/).map(s=>s.trim().toLowerCase().replace(/\.+$/,'')).filter(Boolean))];
async function load(){
  $('retry').disabled=true;$('fields').disabled=true;
  try{
  state=await api();dirty=false;$('resourceMode').value=state.resourceModePreference||'auto';$('language').value=state.language||currentLanguage();
  if(!['exceptions','allowed','blocked'].every(k=>Array.isArray(state[k]))||typeof state.lockedFile!=='boolean')
    throw new Error('PROTECTION_RESPONSE');
  for(const k of booleans)$(k).checked=state[k];
  $('familyEnabled').checked=state.family;$('allowOnly').value=String(state.allowOnly);
  $('downloadDirectory').value=state.downloadDirectory;
  for(const k of ['exceptions','allowed','blocked'])$(k).value=state[k].join('\n');
  $('lists').textContent=t('Local lists: {ads} ad/tracker domains, {adult} adult domains, {violence} violence domains.',{ads:state.adDomains,adult:state.adultDomains,violence:state.violenceDomains});
  if(!state.familyListsLoaded)$('lists').textContent+=' '+t('Family lists load when Family Protection is enabled.');
  $('fields').disabled=state.lockedFile;$('recovery').hidden=!state.lockedFile;
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
    performanceBaseline={sleepEnabled:$('sleepEnabled').checked,sleepMinutes:Number($('sleepMinutes').value),keepAwake:domains('keepAwake')};
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
    const settings={resourceModePreference:$('resourceMode').value,family:$('familyEnabled').checked,allowOnly:$('allowOnly').value==='true',downloadDirectory:$('downloadDirectory').value.trim()};
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
    const extra={settings:changedSettings(state,settings),expectedRevision:state.revision,pin:$('pin').value};if($('newPin').value)extra.newPin=$('newPin').value;
    const result=await api('save',extra);
    const wasFamily=state.family;
    dirty=false;state=result;for(const k of booleans)$(k).checked=state[k];
    for(const k of ['exceptions','allowed','blocked'])$(k).value=state[k].join('\n');
    $('resourceMode').value=state.resourceModePreference||'auto';
    $('pin').value='';$('newPin').value='';$('confirmPin').value='';
    // Enforce a family-mode transition before optional UI preference writes.
    let changed;
    try { changed=await chrome.runtime.sendMessage({op:'settingsChanged',family:result.family,wasFamily}); }
    catch { throw new Error('Native settings were saved. Reopen the browser to finish applying extension controls.'); }
    if(!changed?.ok)throw new Error('Native settings were saved. Reopen the browser to finish applying extension controls.');
    if(performanceReady){try{
      const currentPerformance={sleepEnabled:$('sleepEnabled').checked,sleepMinutes:Number($('sleepMinutes').value),keepAwake};
      await chrome.storage.local.set(changedSettings(performanceBaseline||{},currentPerformance));
      performanceBaseline=currentPerformance;
    }catch(error){message($('performanceStatus'),new Error('Performance preferences could not be saved. Native settings were saved.'));}}
    message($('status'),changed?.error?new Error(t('Native settings saved. {reason}',{reason:errorText(changed.error)})):'Settings saved. Reload sites for protection changes; restart the browser for resource mode changes.');
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

let dirty=false;
$('form').addEventListener?.('input',event=>{if(event.target?.id!=='pin')dirty=true;});
chrome.storage?.onChanged?.addListener((changes,area)=>{
  if(area!=='local'||!changes.protectionRevision||changes.protectionRevision.newValue===state?.revision)return;
  if(dirty)message($('status'),new Error('Settings changed in another window. Reload settings before saving.'));
  else load();
});

$('updateFilters').onclick=async()=>{
  $('updateFilters').disabled=true;
  try {
    if(dirty)throw new Error('Save or reload your settings before updating filters.');
    message($('filterStatus'),'Downloading ad filters…');
    const domains=await downloadAdFilters();
    await api('updateAdFilters',{domains,expectedRevision:state.revision,pin:$('pin').value});
    $('pin').value='';await load();message($('filterStatus'),'Ad filters updated. Reload open sites.');
  } catch(error){message($('filterStatus'),error);}finally{$('updateFilters').disabled=false;}
};
$('restoreBackup').onclick=async()=>{
  $('restoreBackup').disabled=true;
  try {await api('restoreBackup',{pin:$('recoveryPin').value});$('recoveryPin').value='';await load();}
  catch(error){message($('status'),error);}finally{$('restoreBackup').disabled=false;}
};

if(typeof window!=='undefined')window.addEventListener('focus',()=>{if(!dirty&&!$('save').disabled)load();});
