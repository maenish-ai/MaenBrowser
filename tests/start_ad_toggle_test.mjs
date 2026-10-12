import fs from 'node:fs';
import assert from 'node:assert/strict';
const html=fs.readFileSync('assets/companion/start.html','utf8');
const js=fs.readFileSync('assets/companion/start.js','utf8');
assert(!html.includes('adToggle'));
assert(!html.includes('protectionCard'));
assert(!js.includes('api('));
assert(html.includes('id="search"')&&html.includes('https://www.youtube.com/'));
console.log('Start page contains search and shortcuts, no ad toggle or native protection polling');
