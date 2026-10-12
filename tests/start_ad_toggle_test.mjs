import fs from 'node:fs';
import assert from 'node:assert/strict';
const html=fs.readFileSync(new URL('../assets/companion/start.html',import.meta.url),'utf8');
const js=fs.readFileSync(new URL('../assets/companion/start.js',import.meta.url),'utf8');
assert.match(html,/id="adToggle"/);
assert.match(html,/Ad &amp; tracker blocking/);
assert.match(js,/api\('save',\{settings:\{ads:!protectionState\.ads\}\}\)/);
assert.match(js,/textContent=protectionState\.ads\?'ON':'OFF'/);
console.log('start ad toggle contract OK');
