import {checkSecurityData} from './security-updates.js';
import {api,$,message} from './api.js';
import {t} from './i18n.js';
// Optional controls must not prevent the native settings panel from loading.
if(['checkSecurityData','automaticSecurityData','securityDataStatus','pin'].every(id=>document.getElementById(id))){
async function load(){
 try{
  const p=await chrome.storage.local.get({securityDataAutomatic:true,securityDataUpdated:0});
  $('automaticSecurityData').checked=p.securityDataAutomatic;
  const policy=await (await fetch(chrome.runtime.getURL('security-update-policy.json'))).json();
  $('securityDataStatus').textContent=t(policy.enabled&&policy.publicKeySpki?'Signed data updates are configured.':'Publisher signing is not configured. Existing filters remain active.');
 }catch(error){message($('securityDataStatus'),error);}
}
$('automaticSecurityData').onchange=async()=>{try{await chrome.storage.local.set({securityDataAutomatic:$('automaticSecurityData').checked});}catch(error){message($('securityDataStatus'),error);await load();}};
$('checkSecurityData').onclick=async()=>{
 $('checkSecurityData').disabled=true;
 try{
  const result=await checkSecurityData({api,pin:$('pin').value});
  const text={unavailable:'Open MaenBrowser normally or repair protected settings before updating.','up-to-date':'Security data is up to date.',updated:'Signed security data updated.',unconfigured:'Publisher signing is not configured. Existing filters remain active.','parent-required':'Enter the parent PIN before updating protected security data.',disabled:'Automatic security checks are disabled.'};
  message($('securityDataStatus'),text[result.status]||'Existing filters remain active.');
 }catch(error){message($('securityDataStatus'),error);}
 finally{$('pin').value='';$('checkSecurityData').disabled=false;}
};
load();

}
