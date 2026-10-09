import {strict as assert} from 'node:assert';
const sent=[];
globalThis.chrome={runtime:{sendMessage:async request=>{sent.push(request);return {ok:true,data:{ok:true,ads:true,family:false,exceptions:[],language:'ar'}};}},storage:{local:{set:async()=>{}}}};
globalThis.fetch=()=>{throw new Error('A popup must not fetch the native endpoint directly');};
const {api}=await import('../assets/companion/api.js');
assert.equal((await api()).ads,true);
await api('save',{settings:{ads:false},pin:'test-pin'});
assert.deepEqual(sent[1],{op:'nativeApi',operation:'save',extra:{settings:{ads:false},pin:'test-pin'}});
chrome.runtime.sendMessage=async()=>({ok:false,error:'Incorrect parent PIN.'});
await assert.rejects(api('save'),/Incorrect parent PIN/);
console.log('Popup API routes through worker and preserves settings errors');
let fetches=0;
globalThis.fetch=async()=>{fetches++;return {ok:true,json:async()=>({ok:true,ads:false,family:false,exceptions:[],language:'en'})};};
chrome.runtime.sendMessage=async()=>{throw new Error('Worker unavailable');};
assert.equal((await api()).ads,false);assert.equal(fetches,1);
await assert.rejects(api('save',{settings:{ads:true}}),/PROTECTION_WORKER/);
assert.equal(fetches,1,'settings writes must not be replayed');
chrome.runtime.sendMessage=async()=>({ok:false,error:'Incorrect parent PIN.'});
await assert.rejects(api('save'),/Incorrect parent PIN/);assert.equal(fetches,1);
chrome.runtime.sendMessage=async()=>({ok:false,code:'PROTECTION_NETWORK'});
globalThis.fetch=async()=>({ok:false,status:403});
await assert.rejects(api(),/PROTECTION_HTTP_403/);
console.log('Read fallback, no write replay and diagnostic codes passed');

chrome.runtime.sendMessage=async()=>({ok:true,data:{ok:true,ads:true,family:false,exceptions:[],language:'ar'}});
chrome.storage.local.set=async()=>{throw new Error('Storage unavailable');};
assert.equal((await api()).ads,true,'storage failure must not lose native state');
chrome.runtime.sendMessage=async()=>({ok:true,data:{ok:true}});
await assert.rejects(api(),/PROTECTION_RESPONSE/);

chrome.runtime.sendMessage=async()=>({ok:true,data:{ok:true,ads:true,family:false,exceptions:[],language:'en'}});
chrome.storage.local.set=()=>new Promise(()=>{});
assert.equal((await api()).ads,true,'Pending optional storage must not stall protection');
const realSetTimeout=globalThis.setTimeout;
try{
  globalThis.setTimeout=(fn)=>realSetTimeout(fn,0);
  chrome.runtime.sendMessage=()=>new Promise(()=>{});
  globalThis.fetch=async()=>({ok:true,json:async()=>({ok:true,ads:false,family:false,exceptions:[]})});
  assert.equal((await api()).ads,false,'Unresponsive worker must permit read-only fallback');
  await assert.rejects(api('save',{settings:{ads:true}}),/PROTECTION_WORKER/);
}finally{globalThis.setTimeout=realSetTimeout;}
console.log('Unresponsive worker and pending optional storage do not leave controls waiting indefinitely');
