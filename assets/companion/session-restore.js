import {restorableSession} from './tab-policy.js';
export async function restoreSession(id, preferredWindowId=null) {
  const {sessions=[]}=await chrome.storage.local.get('sessions');
  const entry=sessions.find(s=>s.id===id);
  if(!entry)throw new Error('Session not found.');
  return restoreTabList(entry.tabs,preferredWindowId);
}
export async function restoreTabList(rawTabs,preferredWindowId=null){
  const tabs=restorableSession(rawTabs);
  if(!tabs.length)throw new Error('No restorable tabs.');
  const current=preferredWindowId!=null ? await chrome.windows.get(preferredWindowId) : await chrome.windows.getCurrent();
  if(!Number.isInteger(current?.id))throw new Error('No current window available.');
  if(current.type && current.type!=='normal')throw new Error('Restore requires a normal browser window.');
  const already=new Map();
  const prefix=chrome.runtime.getURL('deferred.html')+'#';
  const existing=await chrome.tabs.query({windowId:current.id});
  for(const tab of existing){
    if(tab.incognito)continue;
    let url=tab.url;
    if(typeof url==='string'&&url.startsWith(prefix)){try{url=decodeURIComponent(url.slice(prefix.length));}catch{continue;}}
    const key=JSON.stringify([url,!!tab.pinned]);already.set(key,(already.get(key)||0)+1);
  }
  let restored=0,skipped=0;
  const groups=new Map();
  for(const tab of tabs){
    const key=JSON.stringify([tab.url,tab.pinned]);
    if(already.get(key)>0){already.set(key,already.get(key)-1);skipped++;continue;}
    // Avoid loading all sites simultaneously on low-memory laptops.
    const url=restored===0?tab.url:chrome.runtime.getURL('deferred.html')+'#'+encodeURIComponent(tab.url);
    let created;
    try{created=await chrome.tabs.create({windowId:current.id,active:restored===0,pinned:tab.pinned,url});}
    catch{throw new Error('Some tabs could not be restored. Existing tabs and saved sessions were kept.');}
    if(tab.group&&Number.isInteger(created.id)){
      const g=groups.get(tab.group.key)||{metadata:tab.group,ids:[]};g.ids.push(created.id);groups.set(tab.group.key,g);
    }
    restored++;
  }
  for(const {metadata,ids} of groups.values()){
    try{
    const groupId=await chrome.tabs.group({tabIds:ids});
    // Keep the first active page visible even if its saved group was collapsed.
    await chrome.tabGroups.update(groupId,{title:metadata.title,color:metadata.color,collapsed:false});
    }catch{/* Runtime group support is optional; restored pages remain available. */}
  }
  return {ok:true,restored,skipped};
}
