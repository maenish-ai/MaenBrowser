import {restorableSession} from './tab-policy.js';
export async function restoreSession(id) {
  const {sessions=[]}=await chrome.storage.local.get('sessions');
  const entry=sessions.find(s=>s.id===id);
  if(!entry)throw new Error('Session not found.');
  const tabs=restorableSession(entry.tabs);
  if(!tabs.length)throw new Error('No restorable tabs.');
  const window=await chrome.windows.create({url:tabs[0].url});
  if(tabs[0].pinned&&window.tabs?.[0])await chrome.tabs.update(window.tabs[0].id,{pinned:true});
  // Background entries are tiny local documents. They never fetch the site until selected.
  for(const tab of tabs.slice(1))await chrome.tabs.create({windowId:window.id,active:false,pinned:tab.pinned,
    url:chrome.runtime.getURL('deferred.html')+'#'+encodeURIComponent(tab.url)});
  return {ok:true};
}
