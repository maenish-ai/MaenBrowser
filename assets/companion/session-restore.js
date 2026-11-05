import {restorableSession} from './tab-policy.js';
export async function restoreSession(id, preferredWindowId=null) {
  const {sessions=[]}=await chrome.storage.local.get('sessions');
  const entry=sessions.find(s=>s.id===id);
  if(!entry)throw new Error('Session not found.');
  const tabs=restorableSession(entry.tabs);
  if(!tabs.length)throw new Error('No restorable tabs.');
  const current=preferredWindowId!=null ? await chrome.windows.get(preferredWindowId) : await chrome.windows.getCurrent();
  if(!Number.isInteger(current?.id))throw new Error('No current window available.');
  if(current.type && current.type!=='normal')throw new Error('Restore requires a normal browser window.');
  let restored=0;
  const groups=new Map();
  for(const tab of tabs){
    // Avoid loading all sites simultaneously on low-memory laptops.
    const url=restored===0?tab.url:chrome.runtime.getURL('deferred.html')+'#'+encodeURIComponent(tab.url);
    const created=await chrome.tabs.create({windowId:current.id,active:restored===0,pinned:tab.pinned,url});
    if(tab.group&&Number.isInteger(created.id)){
      const g=groups.get(tab.group.key)||{metadata:tab.group,ids:[]};g.ids.push(created.id);groups.set(tab.group.key,g);
    }
    restored++;
  }
  for(const {metadata,ids} of groups.values()){
    const groupId=await chrome.tabs.group({tabIds:ids});
    // Keep the first active page visible even if its saved group was collapsed.
    await chrome.tabGroups.update(groupId,{title:metadata.title,color:metadata.color,collapsed:false});
  }
  return {ok:true,restored};
}
