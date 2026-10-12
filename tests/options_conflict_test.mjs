import assert from 'node:assert/strict';
import fs from 'node:fs';
import vm from 'node:vm';
import {changedSettings} from '../assets/companion/settings-patch.js';
const source=fs.readFileSync('assets/companion/options.js','utf8').replace(/^import .*;\n/gm,'');
const nodes=new Map();
const $=id=>{if(!nodes.has(id))nodes.set(id,{value:'',checked:false,disabled:false,textContent:''});return nodes.get(id);};
let native={ok:true,revision:'1',ads:true,adult:true,violence:true,safeSearch:true,askDownload:true,httpsOnly:false,
 family:false,lockedFile:false,allowOnly:true,resourceModePreference:'auto',downloadDirectory:'C:\\Downloads',exceptions:[],allowed:[],blocked:[]};
let writes=[];
const ctx=vm.createContext({$,changedSettings,t:s=>s,errorText:String,currentLanguage:()=> 'en',diagnosticCode:e=>e.message,
 console,confirm:()=>true,message:(n,s)=>{n.textContent=String(s);},api:async(op,extra)=>{
 if(op==='save'){
  writes.push(extra);
  if(extra.expectedRevision!==native.revision)throw new Error('Settings changed in another window. Reload settings before saving.');
  native={...native,...extra.settings,revision:String(Number(native.revision)+1)};
 }
 return structuredClone(native);
 },chrome:{storage:{local:{get:async()=>({keepAwake:[],sleepEnabled:true,sleepMinutes:30}),set:async()=>{}}},
 runtime:{sendMessage:async()=>({ok:true})},action:{setBadgeText:async()=>{}}}});
vm.runInContext(source,ctx);await new Promise(r=>setImmediate(r));
// Another popup changes ads after the panel was painted.
native={...native,ads:false,revision:'2'};
$('askDownload').checked=false;
await $('form').onsubmit({preventDefault(){}});
assert.equal(writes.length,1);assert.equal(writes[0].expectedRevision,'1');
assert.deepEqual(writes[0].settings,{askDownload:false});
assert.equal(native.ads,false);assert.equal(native.askDownload,true);
assert.match($('status').textContent,/another window/);
assert.equal(writes.length,1,'Conflicted write must not be replayed');
await vm.runInContext('load()',ctx);
$('askDownload').checked=false;
await $('form').onsubmit({preventDefault(){}});
assert.equal(native.ads,false);assert.equal(native.askDownload,false);
assert.equal(writes[1].expectedRevision,'2');
console.log('Real options submit: stale panel cannot restore ad state; conflict is visible, no retry, explicit reload works');
