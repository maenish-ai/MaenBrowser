import {strict as assert} from 'node:assert';
import {mediaError} from '../assets/companion/media-errors.js';
for(const [input,code] of [[1,'MB-201'],[2,'MB-202'],[3,'MB-203'],[4,'MB-204'],[undefined,'MB-299'],[99,'MB-299']])assert.equal(mediaError(input)[0],code);
console.log('Media classification preserves abort/network/decode/source/unknown distinctions');
