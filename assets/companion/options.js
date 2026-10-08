import {api,$,message} from './api.js';
let state;
const booleans=['ads','adult','violence','safeSearch','askDownload','httpsOnly'];
const domains=id=>[...new Set($(id).value.split(/\s+/).map(s=>s.trim().toLowerCase().replace(/\.+$/,'')).filter(Boolean))];
async function load(){
  state=await api();
  for(const k of booleans)$(k).checked=state[k];
  $('familyEnabled').checked=state.family;$('allowOnly').value=String(state.allowOnly);
  $('downloadDirectory').value=state.downloadDirectory;
  for(const k of ['exceptions','allowed','blocked'])$(k).value=state[k].join('\n');
  const p=await chrome.storage.local.get({sleepEnabled:true,sleepMinutes:30,keepAwake:[]});
  $('sleepEnabled').checked=p.sleepEnabled;$('sleepMinutes').value=String(p.sleepMinutes);$('keepAwake').value=p.keepAwake.join('\n');
  $('lists').textContent=`Local lists: ${state.adDomains.toLocaleString()} advertising/tracking/mixed threat domains, ${state.adultDomains.toLocaleString()} adult domains, ${state.violenceDomains.toLocaleString()} violence domains.`;
  $('fields').disabled=state.lockedFile;
  message($('status'),state.lockedFile?new Error('Protected settings could not be read. Family browsing remains restricted. Restore the profile backup or reinstall with a new profile.'):state.listError?new Error(state.listError):'Native settings loaded.');
}
$('form').onsubmit=async e=>{
  e.preventDefault();$('save').disabled=true;
  try{
    if($('newPin').value!==$('confirmPin').value)throw new Error('New PIN confirmation does not match.');
    const settings={family:$('familyEnabled').checked,allowOnly:$('allowOnly').value==='true',downloadDirectory:$('downloadDirectory').value.trim()};
    for(const k of booleans)settings[k]=$(k).checked;
    for(const k of ['exceptions','allowed','blocked'])settings[k]=domains(k);
    const keepAwake=domains('keepAwake');
    if(keepAwake.some(x=>!/^([a-z0-9-]+\.)+[a-z0-9-]+$/.test(x)))throw new Error('Keep-awake entries must be domain names, not full URLs.');
    if(settings.family&&!state.family&&!confirm('Enabling Family Protection closes existing web tabs and disables other extensions. Save your work first. Continue?'))return;
    const extra={settings,pin:$('pin').value};if($('newPin').value)extra.newPin=$('newPin').value;
    const result=await api('save',extra);
    await chrome.storage.local.set({sleepEnabled:$('sleepEnabled').checked,sleepMinutes:Number($('sleepMinutes').value),keepAwake});
    const changed=await chrome.runtime.sendMessage({op:'settingsChanged',family:result.family,wasFamily:state.family});
    state=result;$('pin').value='';$('newPin').value='';$('confirmPin').value='';
    message($('status'),changed?.error?new Error('Native settings saved. '+changed.error):'Settings saved. Reload open sites for ad-block changes.');
    await chrome.action.setBadgeText({text:state.ads?'ON':'OFF'});
  }catch(err){message($('status'),err);}finally{$('save').disabled=false;}
};
load().catch(e=>message($('status'),e));
