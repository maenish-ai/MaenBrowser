import {strict as assert} from 'node:assert';
import fs from 'node:fs';
import vm from 'node:vm';
const source=fs.readFileSync('assets/companion/report-download.js','utf8').replace('export function','function');
for(const fails of [false,true]){
 const events=[];let blob,anchor,timer;
 const context=vm.createContext({JSON,Blob,URL:{createObjectURL(b){blob=b;return 'blob:test';},revokeObjectURL(u){events.push(u);}},setTimeout(fn){timer=fn;},document:{body:{append(a){anchor=a;}},createElement(){return {click(){events.push('click');if(fails)throw Error('failed');},remove(){events.push('remove');}};}}});
 vm.runInContext(source,context);
 if(fails)assert.throws(()=>context.downloadReport({schema:1},'report.json'));
 else context.downloadReport({schema:1},'report.json');
 assert.equal(anchor.download,'report.json');assert.equal(anchor.href,'blob:test');
 assert.equal(blob.type,'application/json');assert.deepEqual(JSON.parse(await blob.text()),{schema:1});
 assert.deepEqual(events,['click','remove']);timer();assert.equal(events[2],'blob:test');
}
console.log('Report download: JSON, filename and cleanup on success/failure passed');
