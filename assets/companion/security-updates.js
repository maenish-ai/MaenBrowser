// Signed DATA only. No executable updater, arbitrary scriptlets or cookie transfer.
export const SECURITY_ALARM='maen-signed-data-check';
const encoder=new TextEncoder();
function decodeBase64(value,max){
 if(typeof value!=='string'||!value||value.length>max||!/^[A-Za-z0-9+/]*={0,2}$/.test(value))throw Error('Invalid signed update encoding.');
 const raw=atob(value);return Uint8Array.from(raw,c=>c.charCodeAt(0));
}
function version(s){if(!/^\d+\.\d+\.\d+$/.test(s))throw Error('Invalid update version.');const parts=s.split('.').map(Number);if(!parts.every(Number.isSafeInteger))throw Error('Invalid update version.');return parts;}
function compare(a,b){a=version(a);b=version(b);for(let i=0;i<3;i++)if(a[i]!==b[i])return a[i]>b[i]?1:-1;return 0;}
function permittedUrl(value,policy){
 const u=new URL(value);if(u.protocol!=='https:'||u.username||u.password||u.hash||u.origin!==policy.allowedDownloadOrigin||!u.pathname.startsWith(policy.allowedDownloadPrefix))throw Error('Untrusted update download origin.');return u.href;
}
export async function verifySecurityEnvelope(envelope,policy,{installed,acceptedSequence=0,now=Date.now()}={}){
 if(policy.schema!==1||policy.algorithm!=='RSA-PSS-SHA256'||!policy.enabled||!policy.publicKeySpki)throw Error('Publisher signing is not configured.');
 if(envelope?.schema!==1||envelope.algorithm!=='RSA-PSS-SHA256')throw Error('Unsupported signed update format.');
 const payload=decodeBase64(envelope.payload,96*1024),signature=decodeBase64(envelope.signature,2048);
 const key=await crypto.subtle.importKey('spki',decodeBase64(policy.publicKeySpki,4096),{name:'RSA-PSS',hash:'SHA-256'},false,['verify']);
 if(key.algorithm.modulusLength<2048||key.algorithm.modulusLength>4096)throw Error('Unsupported publisher key size.');
 if(!await crypto.subtle.verify({name:'RSA-PSS',saltLength:32},key,signature,payload))throw Error('Update signature rejected.');
 const data=JSON.parse(new TextDecoder('utf-8',{fatal:true}).decode(payload));
 if(data.product!=='MaenBrowser-security-data'||data.schema!==1||!Number.isSafeInteger(data.sequence)||data.sequence<=acceptedSequence||data.sequence<=0)throw Error('Stale or invalid signed update sequence.');
 const issued=Date.parse(data.issuedUtc),expires=Date.parse(data.expiresUtc);
 if(!Number.isFinite(issued)||!Number.isFinite(expires)||issued>now+300000||expires<=now||expires<=issued||expires-issued>31*86400000)throw Error('Signed update is expired or has invalid dates.');
 if(compare(installed,data.minimumAppVersion)<0)throw Error('A newer browser is required for this security data.');
 if(!data.lists||typeof data.lists!=='object'||Array.isArray(data.lists))throw Error('Signed update lists are missing.');
 for(const [name,list] of Object.entries(data.lists)){
  if(!['ads','adult','violence'].includes(name)||!list||!/^[a-f0-9]{64}$/.test(list.sha256)||!Number.isSafeInteger(list.size)||list.size<=0||list.size>4*1024*1024)throw Error('Invalid signed update list.');
  permittedUrl(list.url,policy);
 }
 if(!['ads','adult','violence'].every(k=>data.lists[k]))throw Error('Incomplete signed update.');
 if(Object.values(data.lists).reduce((sum,item)=>sum+item.size,0)>5*1024*1024)throw Error('Combined security data exceeds limit.');
 return data;
}
export async function fetchBounded(url,max,fetcher=fetch){
 const r=await fetcher(url,{credentials:'omit',cache:'no-store',redirect:'error',signal:AbortSignal.timeout(30000)});
 if(!r.ok||!r.body)throw Error('Security update download failed. Previous filters were kept.');
 const reader=r.body.getReader();let length=0;const chunks=[];
 try{for(;;){const {done,value}=await reader.read();if(done)break;length+=value.byteLength;if(length>max)throw Error('Security update exceeds size limit.');chunks.push(value);}}
 finally{await reader.cancel().catch(()=>{});reader.releaseLock();}
 const result=new Uint8Array(length);let offset=0;for(const c of chunks){result.set(c,offset);offset+=c.byteLength;}return result;
}
export async function fetchSecurityList(list,policy,fetcher=fetch){
 const bytes=await fetchBounded(permittedUrl(list.url,policy),list.size,fetcher);
 const sha=[...new Uint8Array(await crypto.subtle.digest('SHA-256',bytes))].map(n=>n.toString(16).padStart(2,'0')).join('');
 if(bytes.byteLength!==list.size||sha!==list.sha256)throw Error('Security data integrity check failed. Previous filters were kept.');
 const text=new TextDecoder('utf-8',{fatal:true}).decode(bytes),lines=text.trimEnd().split('\n');
 const minimum=list.kind==='violence'?100:10000;
 if(lines.length<minimum||lines.length>250000||!text.endsWith('\n')||lines.some((s,i)=>s.length>253||!s.includes('.')||!s.split('.').every(p=>/^[a-z0-9](?:[a-z0-9-]{0,61}[a-z0-9])?$/.test(p))||(i>0&&lines[i-1]>=s)))throw Error('Security data domains are invalid. Previous filters were kept.');
 return text;
}
export async function checkSecurityData({api,storage=chrome.storage.local,fetcher=fetch,automatic=false,pin=''}={}){
 const policy=await (await fetcher(chrome.runtime.getURL('security-update-policy.json'))).json();
 const prefs=await storage.get({securityDataAutomatic:true,securityDataAccepted:0});
 if(automatic&&!prefs.securityDataAutomatic)return {status:'disabled'};
 if(!policy.enabled||!policy.publicKeySpki)return {status:'unconfigured'};
 const feed=new URL(policy.feedUrl);if(feed.protocol!=='https:'||feed.username||feed.password||feed.hash)throw Error('Invalid publisher feed URL.');
 const envelope=JSON.parse(new TextDecoder().decode(await fetchBounded(feed.href,128*1024,fetcher)));
 const native=await api();
 if(native.version!==chrome.runtime.getManifest().version)throw Error('Browser and controls have different versions. Close all windows and reopen the updated installation.');
 if(native.lockedFile||native.securityDataCorrupt||native.agentMode)return {status:'unavailable'};
 const nativeFloor=Number(native.securityDataSequence||0),localFloor=Number(prefs.securityDataAccepted);
 if(!Number.isSafeInteger(nativeFloor)||nativeFloor<0||!Number.isSafeInteger(localFloor)||localFloor<0)throw Error('Invalid security data sequence state.');
 const floor=Math.max(nativeFloor,localFloor);
 const data=await verifySecurityEnvelope(envelope,policy,{installed:chrome.runtime.getManifest().version,acceptedSequence:Math.max(0,floor-1)});
 if(data.sequence===floor&&nativeFloor===floor){await storage.set({securityDataAccepted:floor});return {status:'up-to-date',sequence:floor};}

 // Automatic updates never bypass parent PIN or agent restrictions.
 if((native.family||native.hasPin)&&!pin)return {status:'parent-required',sequence:data.sequence};
 const lists={};
 for(const kind of ['ads','adult','violence'])lists[kind]=await fetchSecurityList({...data.lists[kind],kind},policy,fetcher);
 if(new TextEncoder().encode(JSON.stringify(lists)).byteLength>5*1024*1024)throw Error('Combined security data exceeds limit.');
 await api('updateSecurityData',{lists,sequence:String(data.sequence),pin,expectedRevision:native.revision});
 // Keep sequence only after the native atomic write succeeds. No silent downgrade
 // is allowed by this transport. Native local policy remains authoritative.
 await storage.set({securityDataAccepted:data.sequence,securityDataUpdated:Date.now()});
 return {status:'updated',sequence:data.sequence};
}
export function registerSecurityUpdates(api){
 chrome.notifications?.onClicked?.addListener(id=>{if(id==='maen-security-data')chrome.tabs.create({url:chrome.runtime.getURL('options.html')});});
 chrome.alarms.create(SECURITY_ALARM,{periodInMinutes:24*60}).catch(()=>{});
 chrome.alarms.onAlarm.addListener(async a=>{
  if(a.name!==SECURITY_ALARM)return;
  try{
   const result=await checkSecurityData({api,automatic:true});await chrome.storage.local.set({securityDataStatus:result.status});
   if(result.status==='parent-required'){
    const p=await chrome.storage.local.get(['securityDataNotified','language']);
    if(p.securityDataNotified!==result.sequence&&typeof chrome.notifications?.create==='function'){
     await chrome.notifications.create('maen-security-data',{type:'basic',iconUrl:'icon128.png',title:'MaenBrowser',message:p.language==='ar'?'تتوفر بيانات حماية موقعة جديدة. افتح الإعدادات وأدخل رمز الوالد لتحديثها.':'Signed protection data is available. Open settings and enter the parent PIN to update.'});
     await chrome.storage.local.set({securityDataNotified:result.sequence});
    }
   }
  }
  catch(error){await chrome.storage.local.set({securityDataStatus:'failed'}).catch(()=>{});}
 });
}
