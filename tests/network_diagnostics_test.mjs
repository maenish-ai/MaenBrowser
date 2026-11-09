import {strict as assert} from 'node:assert';
import {describeNetwork} from '../assets/companion/network-diagnostics.js';
const t=(s,v={})=>s.replace(/\{(\w+)\}/g,(_,k)=>String(v[k]??''));
assert.match(describeNetwork({},t),/matching browser/);
assert.match(describeNetwork({networkDiagnosticsVersion:1},t),/No recent/);
assert.match(describeNetwork({networkDiagnosticsVersion:1,recentNetworkAgeSeconds:3,recentNetworkCategory:'proxy'},t),/proxy or tunnel/);
assert.match(describeNetwork({networkDiagnosticsVersion:1,recentNetworkAgeSeconds:301,recentNetworkCategory:'dns'},t),/No recent/);
assert.match(describeNetwork({networkDiagnosticsVersion:1,recentNetworkAgeSeconds:0,recentNetworkCategory:'unknown'},t),/Another page/);
console.log('Network diagnostics: recent, privacy-limited categories and unsupported/stale states');
