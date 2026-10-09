import {strict as assert} from 'node:assert';
import {readFileSync} from 'node:fs';
import vm from 'node:vm';
const source=readFileSync('assets/companion/popup.js','utf8').replace(/^import .*;\n/gm,'');
const nodes=new Map();const $=id=>{if(!nodes.has(id))nodes.set(id,{textContent:'',disabled:false});return nodes.get(id);};
const context=vm.createContext({$,console,revealVideoControls(){},host:()=>'',matchesHost:()=>false,
 t:(s,v={})=>s.replace(/\{(\w+)\}/g,(_,key)=>v[key]||''),message:(el,s)=>{el.textContent=String(s);},
 api:async()=>({ok:true,ads:true,family:false,exceptions:[],version:'1.8.5',blockedRequests:'0'}),
 chrome:{tabs:{query:async()=>{throw new Error('No current window');}},runtime:{getManifest:()=>({version:'1.8.5'})},
 action:{setBadgeText:async()=>{throw new Error('Badge unavailable');}}}});
vm.runInContext(source,context);
await new Promise(resolve=>setImmediate(resolve));
assert.equal($('state').textContent,'ON');assert.equal($('toggle').disabled,false);
assert.equal($('exception').disabled,true);
assert.equal($('status').textContent,'Native protection connected.');
console.log('Popup remains usable when tab lookup and badge update fail');
