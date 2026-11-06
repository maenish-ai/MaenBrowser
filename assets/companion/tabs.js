import {selectTabView} from './tab-view.js';
import {validCheckpoints} from './session-recovery.js';
import {restorableSession} from './tab-policy.js';
import {scheduleRender} from './render-scheduler.js';
import {t as i18nText,errorText} from './i18n.js';
import {$,host,webUrl,message} from './api.js';
let tabs=[],viewLimit=100;
function button(label,action){const b=document.createElement('button');b.type='button';b.className='small';b.textContent=label;b.onclick=()=>Promise.resolve(action()).catch(e=>message($('status'),e));return b;}
async function call(op,extra={}){const r=await chrome.runtime.sendMessage({op,...extra});if(!r?.ok)throw new Error(r?.error||'Action failed');return r;}
async function refresh(){tabs=await chrome.tabs.query($('tabScope').value==='all'?{}:{currentWindow:true});render();await sessions();await recovery();}
function render(){
  $('tabs').replaceChildren();const q=$('query').value.toLowerCase();
  const view=selectTabView(tabs,{query:q,sort:$('tabSort').value,limit:viewLimit});
  $('tabViewCount').textContent=i18nText('Showing {shown} of {total} matching tabs.',{shown:view.tabs.length,total:view.count});
  $('showMoreTabs').hidden=view.tabs.length>=view.count||viewLimit>=500;
  for(const t of view.tabs){
    const row=document.createElement('div');row.className='tab';const info=document.createElement('div');
    const title=document.createElement('div');title.className='tab-title';title.textContent=(t.discarded?'◌ ':'')+(t.pinned?'◆ ':'')+(t.title||i18nText('New tab'));
    const url=document.createElement('div');url.className='tab-url';url.textContent=t.url||'';info.append(title,url);
    info.tabIndex=0;info.setAttribute('role','button');const focus=async()=>{await chrome.windows.update(t.windowId,{focused:true});await chrome.tabs.update(t.id,{active:true});};info.onclick=focus;info.onkeydown=e=>{if(e.key==='Enter')focus();};
    const actions=document.createElement('div');actions.className='actions';
    actions.append(button(t.pinned?'Unpin':'Pin',async()=>{await chrome.tabs.update(t.id,{pinned:!t.pinned});await refresh();}),button('Release',async()=>{await call('release',{tabId:t.id});message($('status'),'Memory released. The tab reloads when selected.');await refresh();}),button('×',async()=>{await chrome.tabs.remove(t.id);await refresh();}));
    row.append(info,actions);$('tabs').append(row);
  }
  if(!$('tabs').children.length)$('tabs').textContent='No matching tabs.';
}
async function sessions(){
  const {sessions=[]}=await chrome.storage.local.get('sessions');$('sessions').replaceChildren();
  for(const s of sessions){const row=document.createElement('div');row.className='tab';const name=document.createElement('span');name.textContent=i18nText('{name} · {count} tabs',{name:s.name,count:s.tabs.length});const actions=document.createElement('div');actions.className='actions';
    actions.append(button('Restore',async()=>{const r=await call('restoreSession',{id:s.id});message($('status'),i18nText('Restored {restored} tabs; {skipped} were already open.',r));}),button('Delete',async()=>{await call('deleteSession',{id:s.id});await refresh();}));row.append(name,actions);$('sessions').append(row);}
  if(!sessions.length)$('sessions').textContent='No sessions saved yet.';
}
$('query').oninput=()=>{viewLimit=100;render();};
$('tabScope').onchange=()=>{viewLimit=100;refresh().catch(e=>message($('status'),e));};
$('tabSort').onchange=render;
$('showMoreTabs').onclick=()=>{viewLimit+=100;render();};
$('restore').onclick=async()=>{try{await chrome.sessions.restore();await refresh();}catch(e){message($('status'),e);}};
$('saveSession').onclick=async()=>{try{const name=prompt(i18nText('Name for this session:'),i18nText('My session'));if(name===null)return;await call('saveSession',{name});await refresh();}catch(e){message($('status'),e);}};
$('group').onclick=async()=>{try{const list=await chrome.tabs.query({currentWindow:true});const groups=new Map();for(const t of list){if(t.pinned||!webUrl(t.url))continue;const h=host(t.url);groups.set(h,[...(groups.get(h)||[]),t.id]);}for(const [title,ids]of groups){if(ids.length<2)continue;const id=await chrome.tabs.group({tabIds:ids});await chrome.tabGroups.update(id,{title,collapsed:false});}await refresh();message($('status'),'Grouped sites with two or more tabs.');}catch(e){message($('status'),e);}};
const schedule=scheduleRender(refresh,e=>message($('status'),e),{minInterval:500});
chrome.tabs.onRemoved.addListener(schedule);chrome.tabs.onCreated.addListener(schedule);
chrome.tabs.onUpdated.addListener((_id,change)=>{if(change.title||change.status==='complete'||change.discarded!==undefined)schedule();});
refresh().catch(e=>message($('status'),e));

document.addEventListener('maen-language',schedule);

$('exportSessions').onclick=async()=>{try{
  const {sessions=[]}=await chrome.storage.local.get('sessions');
  const data={product:'MaenBrowser',schema:1,sessions:sessions.slice(0,20).map(s=>({name:String(s.name||'Session').slice(0,100),tabs:restorableSession(s.tabs)}))};
  const url=URL.createObjectURL(new Blob([JSON.stringify(data,null,2)],{type:'application/json'}));
  const a=document.createElement('a');a.href=url;a.download='MaenBrowser-sessions.json';a.click();setTimeout(()=>URL.revokeObjectURL(url),10000);
}catch(e){message($('status'),e);}};
$('importSessions').onclick=()=>{$('sessionFile').click();};
$('sessionFile').onchange=async()=>{try{
  const file=$('sessionFile').files[0];if(!file)return;
  if(file.size>1024*1024)throw new Error('Session file is too large.');
  const data=JSON.parse(await file.text());
  if(data.product!=='MaenBrowser'||data.schema!==1||!Array.isArray(data.sessions)||data.sessions.length>20)throw new Error('Invalid session file.');
  const imported=data.sessions.map(s=>({id:crypto.randomUUID(),name:String(s.name||'Imported session').slice(0,100),created:Date.now(),tabs:restorableSession(s.tabs)})).filter(s=>s.tabs.length);
  if(!imported.length)throw new Error('No safe web addresses in the file.');
  await call('importSessions',{entries:imported});await refresh();
}catch(e){message($('status'),e);}finally{$('sessionFile').value='';}};
$('recoverSessions').onclick=async()=>{try{
  await call('recoverSessions');await refresh();
}catch(e){message($('status'),e);}};

async function recovery(){
 const p=await chrome.storage.local.get({sessionRecoveryEnabled:true,sessionRecovery:[]});
 $('recoveryEnabled').checked=p.sessionRecoveryEnabled!==false;
 $('recoveryItems').replaceChildren();
 for(const item of await validCheckpoints(p.sessionRecovery)){
  const row=document.createElement('div');row.className='tab';const label=document.createElement('span');
  label.textContent=new Date(item.created).toLocaleString()+' · '+i18nText('{count} tabs',{count:item.tabs.length});
  row.append(label,button(i18nText('Restore'),async()=>{const r=await call('restoreRecovery',{digest:item.digest});message($('status'),i18nText('Restored {restored} tabs; {skipped} were already open.',r));}));$('recoveryItems').append(row);
 }
}
$('recoveryEnabled').onchange=async()=>{try{await chrome.storage.local.set({sessionRecoveryEnabled:$('recoveryEnabled').checked});}catch(e){message($('status'),e);await recovery();}};
$('clearRecovery').onclick=async()=>{try{await chrome.storage.local.remove('sessionRecovery');await recovery();}catch(e){message($('status'),e);}};
