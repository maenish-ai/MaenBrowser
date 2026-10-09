import {revealVideoControls} from './video-controls.js';
import {t} from './i18n.js';
import {api, $, host, matchesHost, message} from './api.js';
let state, tab;
const open = path => chrome.tabs.create({url: chrome.runtime.getURL(path)});
async function refresh(){
  [tab] = await chrome.tabs.query({active:true,currentWindow:true});
  state = await api();
  $('connectionDetail').textContent=t('Panel {panel} · Browser {browser}',{panel:chrome.runtime.getManifest().version,browser:state.version});
  const domain=host(tab?.url);
  $('site').textContent=domain || 'Browser page';
  $('state').textContent=state.ads?'ON':'OFF';
  $('toggle').textContent=state.ads?'Turn ad blocking OFF':'Turn ad blocking ON';
  $('toggle').disabled=false;
  $('exception').checked=state.exceptions.some(d=>matchesHost(domain,d));
  $('exception').disabled=!domain || !/^https?:/.test(tab?.url || '') || state.family;
  $('count').textContent=t('{count} requests blocked this browser session',{count:state.blockedRequests});
  message($('status'),state.family?'Family Protection is ON. Changes need the parent PIN.':state.listError ? 'Filter files are missing or empty. Repair the installation.' : 'Native protection connected.');
  await chrome.action.setBadgeText({text:state.ads?'✓':'—'});
}
$('toggle').onclick=async()=>{
  if(!state){$('toggle').disabled=true;try{await refresh();}catch(e){connectionError(e);}return;}
  if(state.family){await open('options.html#family');return;}
  $('toggle').disabled=true;
  try{await api('save',{settings:{ads:!state.ads}});await refresh();}catch(e){message($('status'),e);$('toggle').disabled=false;}
};
$('exception').onchange=async()=>{
  const domain=host(tab?.url);let exceptions=state.exceptions.filter(d=>!matchesHost(domain,d));
  if($('exception').checked)exceptions.push(domain);
  try{await api('save',{settings:{exceptions}});await refresh();message($('status'),'Saved. Reload the site to apply to already loaded content.');}catch(e){message($('status'),e);}
};
$('settings').onclick=()=>open('options.html');$('tabs').onclick=()=>open('tabs.html');$('downloads').onclick=()=>open('downloads.html');
$('extensions').onclick=()=>chrome.tabs.create({url:'chrome://extensions/'});
$('sidebar').onclick=async()=>{try{if(!chrome.sidePanel?.open)throw new Error('This runtime does not expose a sidebar. Use Tabs & sessions instead.');await chrome.sidePanel.open({windowId:tab.windowId});}catch(e){message($('status'),e);}};
$('reader').onclick=async()=>{try{const r=await chrome.runtime.sendMessage({op:'reader',tabId:tab.id});if(!r?.ok)throw new Error(r?.error||'Reader unavailable.');}catch(e){message($('status'),e);}};
function connectionError(error){
  state=null;$('state').textContent=t('Protection unavailable');
  $('toggle').textContent=t('Retry connection');$('toggle').disabled=false;
  $('exception').disabled=true;
  const code=/^PROTECTION_[A-Z0-9_]+$/.test(error?.code||'')?error.code:'PROTECTION_UNKNOWN';
  $('connectionDetail').textContent=t('Panel {panel} · Error {code}',{panel:chrome.runtime.getManifest().version,code});
  chrome.action.setBadgeText({text:'?'}).catch(()=>{});
  console.warn('Native controls connection',code);
  message($('status'),new Error('Could not connect to protection. Please retry or open Settings.'));
}
refresh().catch(connectionError);

$('videoControls').onclick=async()=>{
  try {
    const [current] = await chrome.tabs.query({active:true,currentWindow:true});
    if(!/^https?:/.test(current?.url||''))throw new Error('Open a normal webpage first.');
    const results=await chrome.scripting.executeScript({target:{tabId:current.id,allFrames:true},func:revealVideoControls});
    const count=results.reduce((total,item)=>total+(Number(item.result)||0),0);
    message($('status'),count?t('Controls enabled for {count} videos. Play the video on the page.',{count}):'No accessible video found. Start the site player and try again.');
  } catch(e) { message($('status'),e); }
};
