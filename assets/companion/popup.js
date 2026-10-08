import {api, $, host, matchesHost, message} from './api.js';
let state, tab;
const open = path => chrome.tabs.create({url: chrome.runtime.getURL(path)});
async function refresh(){
  [tab] = await chrome.tabs.query({active:true,currentWindow:true});
  state = await api();
  const domain=host(tab?.url);
  $('site').textContent=domain || 'Browser page';
  $('state').textContent=state.ads?'ON':'OFF';
  $('toggle').textContent=state.ads?'Turn ad blocking OFF':'Turn ad blocking ON';
  $('toggle').disabled=false;
  $('exception').checked=state.exceptions.some(d=>matchesHost(domain,d));
  $('exception').disabled=!domain || !/^https?:/.test(tab?.url || '') || state.family;
  $('count').textContent=`${state.blockedRequests} requests blocked this browser session`;
  message($('status'),state.family?'Family Protection is ON. Changes need the parent PIN.':state.listError || 'Native protection connected.');
  await chrome.action.setBadgeText({text:state.ads?'ON':'OFF'});
}
$('toggle').onclick=async()=>{
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
refresh().catch(e=>{message($('status'),new Error('Could not connect to native protection. Check installation; protection status is unverified. '+e.message));});
