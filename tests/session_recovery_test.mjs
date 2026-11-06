import assert from 'node:assert/strict';
import {webcrypto} from 'node:crypto';
Object.defineProperty(globalThis,'crypto',{value:webcrypto,configurable:true});
globalThis.chrome={runtime:{getURL:s=>'chrome-extension://trusted/'+s}};
const {checkpointSession,validCheckpoints,recoveryTabs}=await import('../assets/companion/session-recovery.js');
let state={},writes=0,list=[{url:'https://example.org',title:'Work'},{url:'https://private.org',incognito:true},{url:'chrome://settings'}];
const storage={get:async defaults=>({...defaults,...state}),set:async value=>{writes++;Object.assign(state,value);}};
const tabs={query:async()=>list};
assert((await checkpointSession({storage,tabs,now:10})).saved);
assert.equal(state.sessionRecovery[0].tabs.length,1);
state.sessionRecovery[0].tabs=state.sessionRecovery[0].tabs.map(t=>({pinned:t.pinned,title:t.title,url:t.url}));assert.equal((await validCheckpoints(state.sessionRecovery)).length,1);

assert.equal((await checkpointSession({storage,tabs,now:11})).reason,'unchanged');assert.equal(writes,1);
for(let i=0;i<5;i++){list=[{url:'https://example.org/'+i}];await checkpointSession({storage,tabs,now:20+i});}
assert.equal(state.sessionRecovery.length,3);
state.sessionRecovery[0].tabs[0].url='https://tampered.org';assert.equal((await validCheckpoints(state.sessionRecovery)).length,2);
list=[];assert.equal((await checkpointSession({storage,tabs})).reason,'empty');
state.sessionRecoveryEnabled=false;assert.equal((await checkpointSession({storage,tabs})).reason,'disabled');
assert.equal(recoveryTabs([{url:'chrome-extension://trusted/deferred.html#https%3A%2F%2Fexample.org'}])[0].url,'https://example.org');
assert.equal(recoveryTabs([{url:'chrome-extension://trusted/deferred.html#javascript%3Aalert(1)'}]).length,0);
state.sessionRecoveryEnabled=true;list=Array(201).fill({url:'https://example.org'});assert.equal((await checkpointSession({storage,tabs})).reason,'too-many-tabs');
list=Array.from({length:200},(_,i)=>({url:'https://example.org/'+i+'a'.repeat(4000)}));
await assert.rejects(()=>checkpointSession({storage,tabs}),/too large/);
console.log('Recovery: private/internal exclusion, deferred URLs, bounded atomic ring, corruption fallback, deduplication and opt-out passed');

assert.equal(recoveryTabs([{url:'https://example.org?access_token=secret'},{url:'https://example.org?page=2'}]).length,1);
