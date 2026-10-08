import {scheduleRender} from './render-scheduler.js';
import {t as i18nText,errorText} from './i18n.js';
import {$,host,webUrl,message} from './api.js';
let tabs=[];
function button(label,action){const b=document.createElement('button');b.type='button';b.className='small';b.textContent=label;b.onclick=()=>Promise.resolve(action()).catch(e=>message($('status'),e));return b;}
async function call(op,extra={}){const r=await chrome.runtime.sendMessage({op,...extra});if(!r?.ok)throw new Error(r?.error||'Action failed');return r;}
async function refresh(){tabs=await chrome.tabs.query({});render();await sessions();}
function render(){
  $('tabs').replaceChildren();const q=$('query').value.toLowerCase();
  for(const t of tabs.filter(t=>(t.title+' '+t.url).toLowerCase().includes(q))){
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
    actions.append(button('Restore',async()=>{const urls=s.tabs.filter(t=>webUrl(t.url)).map(t=>t.url);if(urls.length)await chrome.windows.create({url:urls});}),button('Delete',async()=>{await chrome.storage.local.set({sessions:sessions.filter(x=>x.id!==s.id)});await refresh();}));row.append(name,actions);$('sessions').append(row);}
  if(!sessions.length)$('sessions').textContent='No sessions saved yet.';
}
$('query').oninput=render;
$('restore').onclick=async()=>{try{await chrome.sessions.restore();await refresh();}catch(e){message($('status'),e);}};
$('saveSession').onclick=async()=>{try{const name=prompt(i18nText('Name for this session:'),i18nText('My session'));if(name===null)return;await call('saveSession',{name});await refresh();}catch(e){message($('status'),e);}};
$('group').onclick=async()=>{try{const list=await chrome.tabs.query({currentWindow:true});const groups=new Map();for(const t of list){if(t.pinned||!webUrl(t.url))continue;const h=host(t.url);groups.set(h,[...(groups.get(h)||[]),t.id]);}for(const [title,ids]of groups){if(ids.length<2)continue;const id=await chrome.tabs.group({tabIds:ids});await chrome.tabGroups.update(id,{title,collapsed:false});}await refresh();message($('status'),'Grouped sites with two or more tabs.');}catch(e){message($('status'),e);}};
const schedule=scheduleRender(refresh,e=>message($('status'),e));
chrome.tabs.onRemoved.addListener(schedule);chrome.tabs.onCreated.addListener(schedule);
chrome.tabs.onUpdated.addListener((_id,change)=>{if(change.title||change.status==='complete'||change.discarded!==undefined)schedule();});
refresh().catch(e=>message($('status'),e));

document.addEventListener('maen-language',schedule);
