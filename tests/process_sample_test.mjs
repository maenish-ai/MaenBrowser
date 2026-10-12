import {strict as assert} from 'node:assert';
import {processSample} from '../assets/companion/process-sample.js';
const a={processId:1,logicalProcessors:4,sampleUptimeSeconds:10,browserProcessCpuSeconds:5};
const b={...a,sampleUptimeSeconds:12,browserProcessCpuSeconds:6};
assert.equal(processSample(a,b).cpuPercent,12.5);
assert.equal(processSample(null,b),null);
for(const patch of [{processId:2},{processId:undefined},{logicalProcessors:0},{logicalProcessors:8},{sampleUptimeSeconds:10.5},{sampleUptimeSeconds:9},{browserProcessCpuSeconds:4},{browserProcessCpuSeconds:NaN},{browserProcessCpuSeconds:100}])
 assert.equal(processSample(a,{...b,...patch}),null);
assert.equal(processSample(a,{...b,browserProcessCpuSeconds:5}).cpuPercent,0);
console.log('CPU samples: normalization, restarts, short intervals and invalid counters passed');
