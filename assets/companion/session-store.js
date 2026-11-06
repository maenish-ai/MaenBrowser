// All manual session mutations run in the one extension worker, in order.
let tail=Promise.resolve();
function checked(value){
 if(!Array.isArray(value)||value.length>20)throw Error('Saved sessions are invalid. Recover the previous saved set.');
 if(new TextEncoder().encode(JSON.stringify(value)).byteLength>1024*1024)throw Error('Saved sessions exceed the storage limit. Export and remove older sessions first.');
 return value;
}
export function commitSessions(transform,storage=chrome.storage.local){
 const result=tail.then(async()=>{
  const p=await storage.get({sessions:[]});const before=checked(p.sessions);
  const after=checked(await transform(before));
  await storage.set({sessions:after,sessionsPrevious:before});return {ok:true};
 });
 tail=result.catch(()=>{});return result;
}
export function recoverSessions(storage=chrome.storage.local){
 const result=tail.then(async()=>{
  const p=await storage.get({sessions:[],sessionsPrevious:[]});const previous=checked(p.sessionsPrevious);
  if(!previous.length)throw Error('No previous saved set.');
  let current=[];try{current=checked(p.sessions);}catch{}
  await storage.set({sessions:previous,sessionsPrevious:current});return {ok:true};
 });tail=result.catch(()=>{});return result;
}
