// Local URL checkpoints, never cookies, POST bodies, form contents or private tabs.
import {restorableSession} from './tab-policy.js';
export const RECOVERY_ALARM='maen-session-checkpoint';
const LIMIT=512*1024;
export function recoveryTabs(tabs){
  if(!Array.isArray(tabs))return [];
  const prefix=globalThis.chrome?.runtime?.getURL('deferred.html')+'#';
  const decoded=tabs.map(tab=>{
    if(typeof tab?.url==='string'&&prefix&&tab.url.startsWith(prefix)){
      try{return {...tab,url:decodeURIComponent(tab.url.slice(prefix.length))};}catch{return null;}
    }return tab;
  });
  const result=[];
  for(const tab of decoded){
    // Avoid automatically persisting obvious one-time login/signed-link tokens.
    if(typeof tab?.url==='string'&&/[?&#](?:access_token|id_token|token|auth|authorization|signature|sig|secure|code|session|sid|key|otp|x-amz-[^=&#]+)=/i.test(tab.url))continue;
    result.push(...restorableSession([tab]));if(result.length>200)break;
  }
  return result;
}
async function digest(tabs){
  const bytes=new TextEncoder().encode(JSON.stringify(tabs));
  if(bytes.byteLength>LIMIT)throw new Error('Session checkpoint is too large. Previous checkpoints were kept.');
  return [...new Uint8Array(await crypto.subtle.digest('SHA-256',bytes))].map(n=>n.toString(16).padStart(2,'0')).join('');
}
export async function validCheckpoints(value){
  if(!Array.isArray(value))return [];
  const valid=[];
  for(const item of value.slice(0,3)){
    try{
      if(item?.schema!==1||!Number.isSafeInteger(item.created)||!Array.isArray(item.tabs)||item.tabs.length>200)continue;
      const tabs=recoveryTabs(item.tabs);
      if(tabs.length&&tabs.length===item.tabs.length&&await digest(tabs)===item.digest)valid.push({...item,tabs});
    }catch{/* A damaged entry must not hide an older valid checkpoint. */}
  }return valid;
}
export async function checkpointSession({storage=chrome.storage.local,tabs=chrome.tabs,now=Date.now()}={}){
  const preferences=await storage.get({sessionRecoveryEnabled:true,sessionRecovery:[]});
  if(!preferences.sessionRecoveryEnabled)return {saved:false,reason:'disabled'};
  const original=await tabs.query({});
  const safe=recoveryTabs(original);
  if(safe.length>200)return {saved:false,reason:'too-many-tabs'};
  // Closing a window or startup with only a new tab must not erase recovery data.
  if(!safe.length)return {saved:false,reason:'empty'};
  const hash=await digest(safe),prior=await validCheckpoints(preferences.sessionRecovery);
  const identity=items=>JSON.stringify(items.map(({url,pinned,group})=>({url,pinned,group})));
  if(prior[0]?.digest===hash || (prior[0]&&identity(prior[0].tabs)===identity(safe)))return {saved:false,reason:'unchanged'};
  const item={schema:1,created:now,digest:hash,tabs:safe};
  await storage.set({sessionRecovery:[item,...prior].slice(0,3)});
  return {saved:true,tabs:safe.length};
}
export function registerRecovery(){
  let scheduled=false;
  const schedule=()=>{
    if(scheduled)return;
    scheduled=true;
    chrome.alarms.create(RECOVERY_ALARM,{delayInMinutes:1}).catch(()=>{scheduled=false;});
  };
  chrome.tabs.onCreated.addListener(schedule);chrome.tabs.onRemoved.addListener(schedule);
  chrome.tabs.onUpdated.addListener((_id,change)=>{if(change.url||change.pinned!==undefined||change.groupId!==undefined)schedule();});
  chrome.tabs.onMoved?.addListener(schedule);
  chrome.alarms.onAlarm.addListener(async alarm=>{
    if(alarm.name!==RECOVERY_ALARM)return;
    scheduled=false;
    try{await checkpointSession();}catch{/* Existing snapshots survive a storage or query failure. */}
  });
  // A service worker restart may find an existing alarm. Creating the same name
  // once is harmless; subsequent events do not keep postponing the checkpoint.
  schedule();
}
