import {strict as assert} from 'node:assert';
import {readFileSync} from 'node:fs';
import vm from 'node:vm';
const source=readFileSync('assets/companion/options.js','utf8').replace(/^import .*;\n/gm,'');
const nodes=new Map();
const $=id=>{if(!nodes.has(id))nodes.set(id,{value:'',textContent:'',disabled:false,checked:false});return nodes.get(id);};
let saved=[];
let state={ok:true,ads:true,family:false,hasPin:true,lockedFile:false,allowOnly:true,downloadDirectory:'C:\\Downloads',exceptions:[],allowed:[],blocked:[]};
const context=vm.createContext({$,console,Error,confirm:()=>true,currentLanguage:()=> 'en',t:s=>s,errorText:String,
  message:(node,text)=>node.textContent=String(text),
  api:async(op,extra)=>{if(op==='save'){saved.push(extra);state={...state,...extra.settings};}return {...state};},
  chrome:{storage:{local:{get:async()=>({keepAwake:[],sleepMinutes:30}),set:async()=>{}}},
    runtime:{sendMessage:async()=>({ok:true})},action:{setBadgeText:async()=>{}}}});
vm.runInContext(source,context);await new Promise(resolve=>setImmediate(resolve));
$('ads').checked=false;
await $('form').onsubmit({preventDefault(){}});
assert.equal(saved.length,1);
assert.equal(saved[0].settings.ads,false);
assert.equal(Object.hasOwn(saved[0].settings,'family'),false,'Unchanged family mode must not request a privileged family write');
assert.equal(saved[0].pin,'');
$('familyEnabled').checked=true;$('pin').value='parent-test-pin';
await $('form').onsubmit({preventDefault(){}});
assert.equal(saved[1].settings.family,true);assert.equal(saved[1].pin,'parent-test-pin');
$('ads').checked=true;$('pin').value='parent-test-pin';
await $('form').onsubmit({preventDefault(){}});
assert.equal(saved[2].pin,'parent-test-pin','Active family mode must forward the entered PIN');
$('familyEnabled').checked=false;$('pin').value='parent-test-pin';
await $('form').onsubmit({preventDefault(){}});
assert.equal(saved[3].settings.family,false);assert.equal(saved[3].pin,'parent-test-pin');
console.log('Settings save: ordinary changes omit unchanged family mode; real transitions and PINs are preserved');
