import {strict as assert} from 'node:assert';
import {readFileSync} from 'node:fs';
import vm from 'node:vm';
import {diagnosticCode,describeError} from '../assets/companion/diagnostics.js';
const source=readFileSync('assets/companion/popup.js','utf8').replace(/^import .*;\n/gm,'');
const nodes=new Map();const $=id=>{if(!nodes.has(id))nodes.set(id,{textContent:'',disabled:false,classList:{add(c){this[c]=true;}},select(){this.selected=true;}});return nodes.get(id);};
const context=vm.createContext({$,console,Error,diagnosticCode,describeError,diagnosticReport:()=>'{"code":"E101"}',
  revealVideoControls(){},host:()=>'',matchesHost:()=>false,t:(s,v={})=>s.replace(/\{(\w+)\}/g,(_,k)=>v[k]||''),message(){},
  api:async()=>{throw new Error('Unknown action');},
  chrome:{runtime:{getManifest:()=>({version:'1.8.8'})},action:{setBadgeText:async()=>{}}}});
vm.runInContext(source,context);await new Promise(resolve=>setImmediate(resolve));
assert.match($('status').textContent,/E101/);assert.equal($('status').classList.error,true);
assert.match($('connectionDetail').textContent,/E101/);assert.equal($('toggle').disabled,false);
$('diagnosticReport').onclick();assert.equal($('report').hidden,false);assert.equal($('report').selected,true);
console.log('Popup failure visibly reports numbered error, retry and selectable diagnostic report');
