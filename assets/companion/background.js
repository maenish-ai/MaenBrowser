import {nativeApi as api,webUrl,NativeConnectionError} from './api.js';
import {candidate,restorableSession} from './tab-policy.js';
async function updateAlarm(){
  const {sleepEnabled=true}=await chrome.storage.local.get('sleepEnabled');
  if(sleepEnabled)await chrome.alarms.create('safe-idle-tabs',{periodInMinutes:2});
  else await chrome.alarms.clear('safe-idle-tabs');
}
async function initialize(){
  await updateAlarm();
  try{const s=await api();await chrome.action.setBadgeText({text:s.ads?'✓':'—'});if(s.family)await enforceFamily(true,false);}catch{await chrome.action.setBadgeText({text:'?'});}
}
chrome.runtime.onInstalled.addListener(()=>initialize());
chrome.runtime.onStartup.addListener(()=>initialize());
chrome.storage.onChanged.addListener((changes,area)=>{if(area==='local'&&changes.sleepEnabled)updateAlarm();});
async function pageIsSafe(tabId){
  let timeout;
  try{
    const results=await Promise.race([chrome.scripting.executeScript({target:{tabId,allFrames:true},func:async()=>{
      if(document.visibilityState==='visible'||document.hasFocus())return false;
      if([...document.querySelectorAll('audio,video')].some(e=>!e.paused||e.srcObject))return false;
      if(document.querySelector('[contenteditable="true"],iframe[src*="meet"],iframe[src*="call"]'))return false;
      if([...document.querySelectorAll('input,textarea,select')].some(e=>!['hidden','button','submit','reset'].includes(e.type)&&(e.value||e.checked)))return false;
      // Granted media permissions are treated conservatively even if we cannot
      // observe a site's private MediaStream/RTCPeerConnection objects.
      try{for(const name of ['microphone','camera']){const p=await navigator.permissions.query({name});if(p.state==='granted')return false;}}catch{return false;}
      return true;
    }}),new Promise(resolve=>{timeout=setTimeout(()=>resolve([]),5000);})]);
    return results.length>0&&results.every(r=>r.result===true);
  }catch{return false;}finally{clearTimeout(timeout);}
}
async function releaseTab(id,manual=false,minutes=15){
  if((await chrome.downloads.search({state:'in_progress',limit:1})).length)return {ok:false,error:'A download is running. Tabs are kept awake.'};
  const tab=await chrome.tabs.get(id);const {keepAwake=[]}=await chrome.storage.local.get('keepAwake');
  const probe=manual?{...tab,lastAccessed:0}:tab;
  if(!candidate(probe,Date.now(),minutes,keepAwake)||!await pageIsSafe(id))return {ok:false,error:'This tab is active or may contain work, media or protected content. It was kept awake.'};
  const current=await chrome.tabs.get(id);
  const latest=await chrome.storage.local.get({sleepEnabled:true,keepAwake:[],sleepMinutes:30});
  const recheck=manual?{...current,lastAccessed:0}:current;
  if((!manual&&!latest.sleepEnabled)||!candidate(recheck,Date.now(),manual?15:Math.max(minutes,latest.sleepMinutes),latest.keepAwake)||current.url!==tab.url||current.lastAccessed!==tab.lastAccessed)return {ok:false,error:'Tab changed; it was kept awake.'};
  if((await chrome.downloads.search({state:'in_progress',limit:1})).length)return {ok:false,error:'A download is running. Tabs are kept awake.'};
  await chrome.tabs.discard(id);return {ok:true};
}
chrome.alarms.onAlarm.addListener(async alarm=>{
  if(alarm.name!=='safe-idle-tabs')return;
  try{
    const p=await chrome.storage.local.get({sleepEnabled:true,sleepMinutes:30,keepAwake:[]});if(!p.sleepEnabled)return;
    if((await chrome.downloads.search({state:'in_progress',limit:1})).length)return;
    const tabs=await chrome.tabs.query({});if(!tabs.length)return;
    const {scanOffset=0}=await chrome.storage.session.get('scanOffset');
    let inspected=0,visited=0;
    for(;visited<tabs.length&&inspected<3;visited++){
      const tab=tabs[(scanOffset+visited)%tabs.length];
      if(candidate(tab,Date.now(),p.sleepMinutes,p.keepAwake)){inspected++;await releaseTab(tab.id,false,p.sleepMinutes);}
    }
    await chrome.storage.session.set({scanOffset:(scanOffset+visited)%tabs.length});
  }catch{/* Best effort: inspection errors must never cause a discard. */}
});
async function enforceFamily(enabled,closeTabs){
  if(enabled){
    const all=await chrome.management.getAll();
    const {familyDisabled=[]}=await chrome.storage.local.get('familyDisabled');const ids=new Set(familyDisabled);
    for(const ext of all){if(ext.id!==chrome.runtime.id&&ext.enabled&&ext.type==='extension'){await chrome.management.setEnabled(ext.id,false);ids.add(ext.id);}}
    await chrome.storage.local.set({familyDisabled:[...ids]});
    if(closeTabs){const tabs=await chrome.tabs.query({});const ids=tabs.filter(t=>!String(t.url||'').startsWith(chrome.runtime.getURL(''))).map(t=>t.id);if(ids.length)await chrome.tabs.remove(ids);}
  }
  // Never silently re-enable third-party extensions; the parent chooses them.
}
chrome.management.onEnabled.addListener(async ext=>{try{const s=await api();if(s.family&&ext.id!==chrome.runtime.id)await chrome.management.setEnabled(ext.id,false);}catch{}});
async function reader(tabId){
  const tab=await chrome.tabs.get(tabId);if(!webUrl(tab.url))throw new Error('Open a normal webpage first.');
  const [{result}]=await chrome.scripting.executeScript({target:{tabId},func:()=>({title:document.title,text:(document.querySelector('article')||document.querySelector('main')||document.body).innerText.slice(0,300000),url:location.href})});
  const key='reader-'+crypto.randomUUID();await chrome.storage.session.set({[key]:result});
  await chrome.tabs.create({url:chrome.runtime.getURL('reader.html#'+key)});return {ok:true};
}
chrome.runtime.onMessage.addListener((request,sender,respond)=>{
  if(sender.id!==chrome.runtime.id||!sender.url?.startsWith(chrome.runtime.getURL('')))return false;
  (async()=>{
    if(request.op==='nativeApi'){
      if(!['get','save','setLanguage'].includes(request.operation))return {ok:false,error:'Unknown operation',code:'E106'};
      return {ok:true,data:await api(request.operation,request.extra||{})};
    }
    if(request.op==='release')return releaseTab(request.tabId,true);
    if(request.op==='reader')return reader(request.tabId);
    if(request.op==='settingsChanged'){
      const s=await api();await enforceFamily(s.family,s.family&&!request.wasFamily);await updateAlarm();return {ok:true};
    }
    if(request.op==='saveSession'){
      const tabs=await chrome.tabs.query({currentWindow:true});const {sessions=[]}=await chrome.storage.local.get('sessions');
      const saved={id:crypto.randomUUID(),name:String(request.name||'Saved session').slice(0,100),created:Date.now(),tabs:restorableSession(tabs)};
      await chrome.storage.local.set({sessions:[saved,...sessions].slice(0,20)});return {ok:true};
    }
    return {ok:false,error:'Unknown action',code:'E101'};
  })().then(respond).catch(e=>respond({ok:false,error:e.message,...(e instanceof NativeConnectionError?{code:e.code}:{})}));return true;
});
chrome.commands.onCommand.addListener(command=>{if(command==='open-tabs')chrome.tabs.create({url:chrome.runtime.getURL('tabs.html')});});

chrome.storage.onChanged.addListener((changes,area)=>{
  if(area==='local'&&changes.language)chrome.action.setTitle({title:changes.language.newValue==='ar'?'حماية معن براوزر':'MaenBrowser protection'});
});
