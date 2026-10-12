import {webUrl} from './tab-policy.js';
let target='';try{target=decodeURIComponent(location.hash.slice(1));}catch{}
async function activate(){
  if(document.hidden||!webUrl(target))return;
  const tab=await chrome.tabs.getCurrent();
  if(tab?.active)location.replace(target);
}
document.getElementById('open').onclick=()=>{if(webUrl(target))location.replace(target);};
document.addEventListener('visibilitychange',()=>activate().catch(()=>{}));
activate().catch(()=>{});
