import {strict as assert} from 'node:assert';
import fs from 'node:fs';
import vm from 'node:vm';
import {candidate,restorableSession} from '../assets/companion/tab-policy.js';
const source=fs.readFileSync('assets/companion/background.js','utf8').replace(/^import .*;\n/gm,'');
const event={addListener(){}};
async function check({change={},newDownload=false,settings={}}={}){
 const tab={id:1,url:'https://example.com',status:'complete',lastAccessed:Date.now()-40*60000};
 let reads=0,downloads=0,discarded=false;
 const chrome={runtime:{onInstalled:event,onStartup:event,onMessage:event},commands:{onCommand:event},management:{onEnabled:event},alarms:{onAlarm:event},storage:{onChanged:event,local:{get:async defaults=>({...(typeof defaults==='object'?defaults:{}),...settings})}},tabs:{get:async()=>({...tab,...(++reads>1?change:{})}),discard:async()=>{discarded=true;}},downloads:{search:async()=>++downloads>1&&newDownload?[{}]:[]}};
 const ctx=vm.createContext({chrome,candidate,restorableSession,console,setTimeout,clearTimeout});
 vm.runInContext(source,ctx);vm.runInContext('pageIsSafe=async()=>true',ctx);
 const result=await vm.runInContext('releaseTab(1,false,30)',ctx);
 return {result,discarded};
}
assert((await check()).discarded);
for(const change of [{active:true},{autoDiscardable:false},{lastAccessed:Date.now()},{discarded:true},{url:'https://other.test'}])assert(!(await check({change})).discarded);
assert(!(await check({newDownload:true})).discarded);
assert(!(await check({settings:{sleepEnabled:false}})).discarded);
assert(!(await check({settings:{keepAwake:['example.com']}})).discarded);
assert(!(await check({settings:{sleepMinutes:60}})).discarded);
console.log('Idle discard rechecks: activity, policy, navigation, downloads and longer timeout passed');
