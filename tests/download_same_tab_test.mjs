import {strict as assert} from 'node:assert';
import fs from 'node:fs';import vm from 'node:vm';
const source=fs.readFileSync('assets/companion/download-same-tab.js','utf8');
let handler, timers=[];
class Element { constructor(a){this.a=a} closest(){return this.a} }
const location=new URL('https://example.com/page');
const document={baseURI:location.href,addEventListener:(name,fn)=>{assert.equal(name,'click');handler=fn}};
vm.runInNewContext(source,{document,location,URL,Element,setTimeout:f=>timers.push(f)});
function run(href,extras={}) {let target='_blank';const a={href,get target(){return target},getAttribute:()=>target,setAttribute:(_,v)=>{target=v}};const ev={target:new Element(a),isTrusted:true,defaultPrevented:false,button:0,ctrlKey:false,metaKey:false,shiftKey:false,altKey:false,...extras};handler(ev);return {get target(){return target},a};}
let x=run('https://example.com/file.zip');assert.equal(x.target,'_self');timers.shift()();assert.equal(x.target,'_blank');
x=run('https://other.example/file');assert.equal(x.target,'_blank');
x=run('https://example.com/file',{ctrlKey:true});assert.equal(x.target,'_blank');
x=run('https://example.com/file',{isTrusted:false});assert.equal(x.target,'_blank');
x=run('https://example.com/file');x.a.setAttribute('target','named');timers.shift()();assert.equal(x.target,'named');
console.log('Explicit same-origin downloads: original click retained, modified/cross-origin clicks untouched, target restored');
