import assert from 'node:assert/strict';
import {selectTabView} from '../assets/companion/tab-view.js';
const tabs=[{title:'B',url:'https://b.org',lastAccessed:2},{title:'A',url:'https://a.org',lastAccessed:1},{title:'Private',incognito:true}];
assert.equal(selectTabView(tabs,{sort:'title'}).tabs[0].title,'A');assert.equal(selectTabView(tabs,{sort:'recent'}).tabs[0].title,'B');
assert.equal(selectTabView(tabs,{query:'A.ORG'}).count,1);assert.equal(selectTabView(Array(1000).fill(tabs[0]),{}).tabs.length,100);
assert.equal(selectTabView(tabs,{}).count,2);assert.equal(tabs[0].title,'B');
console.log('Tab view: scoped input, search, sorting, private exclusion and bounded rendering passed');
