import {strict as assert} from 'node:assert';
import {readFileSync} from 'node:fs';
import vm from 'node:vm';
const source=readFileSync('assets/companion/options.js','utf8').replace(/^import .*;\n/gm,'');
const tick=()=>new Promise(resolve=>setImmediate(resolve));
async function run({storageFails=false,locked=false,nativeFails=false}={}){
  const nodes=new Map();
  const $=id=>{if(!nodes.has(id))nodes.set(id,{value:'',textContent:'',disabled:id==='fields',checked:false});return nodes.get(id);};
  let unavailable=nativeFails;
  const native={ads:true,family:false,lockedFile:locked,exceptions:[],allowed:[],blocked:[],language:'en'};
  const context=vm.createContext({$,console,Error,currentLanguage:()=> 'en',t:s=>s,errorText:String,
    message:(node,text)=>node.textContent=String(text),
    api:async()=>{if(unavailable)throw new Error('PROTECTION_NETWORK');return native;},
    chrome:{storage:{local:{get:async()=>{if(storageFails)throw new Error('Storage unavailable');return {keepAwake:'malformed old preference',sleepMinutes:'bad'};}}}}});
  vm.runInContext(source,context);await tick();
  return {$,retry:async()=>{unavailable=false;$('retry').onclick();await tick();}};
}
let test=await run({storageFails:true});
assert.equal(test.$('fields').disabled,false,'Storage failure must not disable native settings');
assert.equal(test.$('ads').checked,true);
assert.equal(test.$('sleepEnabled').disabled,true,'Unavailable preferences must not overwrite stored values');
test=await run();assert.equal(test.$('fields').disabled,false);assert.equal(test.$('sleepMinutes').value,'30');
test=await run({locked:true});assert.equal(test.$('fields').disabled,true,'Corrupt protected profile must remain locked');
test=await run({nativeFails:true});assert.equal(test.$('fields').disabled,true);
assert.equal(test.$('details').textContent,'PROTECTION_NETWORK');
await test.retry();assert.equal(test.$('fields').disabled,false);assert.equal(test.$('details').textContent,'');
console.log('Settings: optional storage isolation, malformed preferences, protected lock and connection retry passed');
