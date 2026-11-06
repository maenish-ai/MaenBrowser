import assert from 'node:assert/strict';
import {downloadAdvice,resumeOriginalDownload} from '../assets/companion/download-recovery.js';
let item={id:1,state:'interrupted',canResume:true},calls=[];
const downloads={search:async q=>{assert.equal(q.id,1);return [item];},resume:async id=>calls.push(id),download:()=>{throw Error('Never replay a URL');}};
await resumeOriginalDownload(1,downloads);assert.deepEqual(calls,[1]);item.canResume=false;await assert.rejects(()=>resumeOriginalDownload(1,downloads));
item={state:'complete',canResume:true};await assert.rejects(()=>resumeOriginalDownload(1,downloads));
assert(downloadAdvice('SERVER_UNAUTHORIZED').includes('POST'));assert(downloadAdvice('FILE_BLOCKED').includes('Do not disable'));
console.log('Download recovery: rechecks resumability, retains original download identity and never replays URL/POST or bypasses safety');
