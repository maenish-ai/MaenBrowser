import {strict as assert} from 'node:assert';
import fs from 'node:fs';
import {translations} from '../assets/companion/translations.js';
for(const [key,pair] of Object.entries(translations)){
  assert.equal(pair.length,2,key);
  assert(pair.every(x=>typeof x==='string'&&x.length>0),key);
  assert(!/[\u0600-\u06ff]/u.test(pair[0]),'Arabic leaks into English: '+key);
  const variables=text=>[...text.matchAll(/\{(\w+)\}/g)].map(x=>x[1]).sort();
  assert.deepEqual(variables(pair[0]),variables(pair[1]),'Translation placeholders: '+key);
}
const en=JSON.parse(fs.readFileSync('assets/companion/_locales/en/messages.json','utf8'));
const ar=JSON.parse(fs.readFileSync('assets/companion/_locales/ar/messages.json','utf8'));
assert.deepEqual(Object.keys(en),Object.keys(ar));
for(const value of Object.values(en))assert(!/[\u0600-\u06ff]/u.test(value.message));
console.log('English/Arabic catalogues and interpolation placeholders verified');
