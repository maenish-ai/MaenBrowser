import {strict as assert} from 'node:assert';
import {restorableSession} from '../assets/companion/tab-policy.js';
const group={key:7,title:'Work',color:'blue',collapsed:true};
const original=[{url:'https://example.org/',group},{url:'https://chat.example.org',group},{url:'https://private.example',incognito:true}];
assert.equal(restorableSession(null).length,0);
assert.equal(restorableSession([{url:'https://user:password@example.org'}]).length,0);
assert.equal(restorableSession([{url:'https://example.org',group:{key:1,color:'invalid'}}])[0].group.color,'grey');
assert(!restorableSession([{url:'https://example.org',pinned:true,group}])[0].group);
assert.equal(restorableSession(Array(250).fill(original[0])).length,200);
const created=[],grouped=[],updates=[];
globalThis.chrome={
 storage:{local:{get:async()=>({sessions:[{id:'test',tabs:original}]})}},
 runtime:{getURL:p=>'chrome-extension://trusted/'+p},
 windows:{get:async id=>({id,type:'normal'}),getCurrent:async()=>({id:42,type:'normal'})},
 tabs:{create:async options=>{created.push(options);return{id:created.length};},group:async options=>{grouped.push(options);return 9;}},
 tabGroups:{update:async(...args)=>updates.push(args)}
};
const {restoreSession}=await import('../assets/companion/session-restore.js');
assert.deepEqual(await restoreSession('test'),{ok:true,restored:2});
assert.equal(created[0].url,'https://example.org/');assert.equal(created[0].active,true);
assert(created[1].url.startsWith('chrome-extension://trusted/deferred.html#'));
assert.equal(created[1].active,false);assert.deepEqual(grouped,[{tabIds:[1,2]}]);
assert.equal(updates[0][1].title,'Work');assert.equal(updates[0][1].collapsed,false);
assert(created.every(t=>t.windowId===42));
await assert.rejects(()=>restoreSession('missing'));
console.log('Sessions: safe imports, bounds, private exclusion, group restore and lazy loading passed');
