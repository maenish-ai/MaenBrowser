export function webUrl(value){try{return ['http:','https:'].includes(new URL(value).protocol);}catch{return false;}}
const protectedHosts=['whatsapp.com','meet.google.com','zoom.us','teams.microsoft.com','teams.live.com','discord.com','slack.com','messenger.com','telegram.org','chat.google.com','outlook.office.com','outlook.office365.com','mail.google.com','adobe.com','figma.com','photopea.com'];
export function candidate(tab, now, minutes, keepAwake=[]){
  if(!Number.isInteger(tab.id)||tab.active||tab.pinned||tab.audible||tab.discarded||tab.incognito||tab.status!=='complete'||tab.autoDiscardable===false)return false;
  if(!Number.isFinite(tab.lastAccessed)||now-tab.lastAccessed<Math.max(5,minutes)*60000)return false;
  let url;try{url=new URL(tab.url);}catch{return false;}
  if(!['http:','https:'].includes(url.protocol))return false;
  // Playback in the native Windows surface is invisible to CEF DOM inspection.
  if(url.protocol==='https:' && /\.(mp4|m4v|m4a|aac)$/i.test(url.pathname))return false;
  const domain=url.hostname.toLowerCase();
  if(protectedHosts.some(h=>domain===h||domain.endsWith('.'+h)))return false;
  if(Array.isArray(keepAwake)&&keepAwake.some(h=>typeof h==='string'&&(domain===h||domain.endsWith('.'+h))))return false;
  return true;
}
export function restorableSession(tabs){
  if(!Array.isArray(tabs))return [];
  return tabs.filter(t=>t&&typeof t.url==='string'&&t.url.length<=8192&&!t.incognito&&webUrl(t.url)&&!new URL(t.url).username&&!new URL(t.url).password).slice(0,200).map(t=>{
    const entry={url:t.url,title:String(t.title||t.url).slice(0,300),pinned:!!t.pinned};
    if(!entry.pinned&&t.group&&Number.isInteger(t.group.key)&&t.group.key>=0){
      const colors=['grey','blue','red','yellow','green','pink','purple','cyan','orange'];
      entry.group={key:t.group.key,title:String(t.group.title||'').slice(0,100),color:colors.includes(t.group.color)?t.group.color:'grey',collapsed:!!t.group.collapsed};
    }
    return entry;
  });
}
