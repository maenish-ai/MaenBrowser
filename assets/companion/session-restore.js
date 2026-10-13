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
  for(const tab of tabs){
    // Avoid loading all sites simultaneously on low-memory laptops.
    const url=restored===0?tab.url:chrome.runtime.getURL('deferred.html')+'#'+encodeURIComponent(tab.url);
    await chrome.tabs.create({windowId:current.id,active:restored===0,pinned:tab.pinned,url});
    restored++;
  }
  return {ok:true,restored};
}
