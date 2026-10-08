export function webUrl(value){try{return ['http:','https:'].includes(new URL(value).protocol);}catch{return false;}}
export function candidate(tab, now, minutes, keepAwake=[]){
  if(!Number.isInteger(tab.id)||tab.active||tab.pinned||tab.audible||tab.discarded||tab.incognito||tab.status!=='complete'||tab.autoDiscardable===false||!webUrl(tab.url))return false;
  const domain=new URL(tab.url).hostname.toLowerCase();
  const protectedHosts=['whatsapp.com','meet.google.com','zoom.us','teams.microsoft.com','teams.live.com',...keepAwake];
  if(protectedHosts.some(h=>domain===h||domain.endsWith('.'+h)))return false;
  if(!Number.isFinite(tab.lastAccessed))return false;
  return now-tab.lastAccessed>=Math.max(15,minutes)*60000;
}
export function restorableSession(tabs){return tabs.filter(t=>!t.incognito&&webUrl(t.url)).slice(0,200).map(t=>({url:t.url,title:String(t.title||t.url).slice(0,300),pinned:!!t.pinned}));}
