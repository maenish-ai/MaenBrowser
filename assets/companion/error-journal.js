// Each writer serializes its own updates; cross-context writes are best-effort.
// Only bounded, structured codes persist. Never store URLs, file names or raw errors.
const key='maenErrorJournal';
let queue=Promise.resolve();
export function sanitizeEvents(value){
  if(!Array.isArray(value))return [];
  return value.filter(e=>e&&typeof e.time==='string'&&/^\d{4}-\d\d-\d\dT[\d:.]+Z$/.test(e.time)&&/^(E\d{3}|MB-\d{3})$/.test(e.code)&&['protection','media'].includes(e.component)).slice(-50).map(({time,code,component})=>({time,code,component}));
}
export async function readErrorJournal(){
  try{return sanitizeEvents((await chrome.storage.local.get(key))[key]);}catch{return [];}
}
export function appendError(code,component){
  const event=sanitizeEvents([{time:new Date().toISOString(),code,component}])[0];
  if(!event)return Promise.resolve();
  queue=queue.then(async()=>{const events=await readErrorJournal();events.push(event);await chrome.storage.local.set({[key]:events.slice(-50)});}).catch(()=>{});
  return queue;
}
