import {strict as assert} from 'node:assert';
import {readFileSync} from 'node:fs';
import vm from 'node:vm';
import {revealVideoControls} from '../assets/companion/video-controls.js';
// The serialized function must not depend on imports or touch sources/playback.
const video={controls:false,src:'blob:https://example.test/local',paused:true,
  controlsList:new Set(['nofullscreen','noplaybackrate'])};
video.controlsList.remove=(...values)=>values.forEach(v=>video.controlsList.delete(v));
const context={document:{querySelectorAll:()=>[video]}};
assert.equal(vm.runInNewContext(`(${revealVideoControls.toString()})()`,context),1);
assert(video.controls);assert(video.paused);assert.equal(video.src,'blob:https://example.test/local');
assert.equal(video.controlsList.size,0);
assert.equal(vm.runInNewContext(`(${revealVideoControls.toString()})()`,{document:{querySelectorAll:()=>[]}}),0);
// Small DOM harness for player behavior, not a claim of codec playback.
class Element {
  constructor(){this.children=[];this.style={};this.listeners={};this.playbackRate=1;}
  append(...children){this.children.push(...children);}
  addEventListener(type,fn){this.listeners[type]=fn;}
  setAttribute(name,value){this[name]=value;}
}
const script=readFileSync(new URL('../assets/companion/direct-player.js',import.meta.url),'utf8');
function player(lang,contentType='video/mp4'){
  const media=new Element(),body=new Element();
  const document={contentType,documentElement:{lang},body,querySelector:()=>media,
    getElementById:id=>body.children.find(e=>e.id===id),createElement:()=>new Element()};
  vm.runInNewContext(script,{document});return {document,media,body};
}
for(const lang of ['en','ar']){
  const {document,media,body}=player(lang);
  assert(media.controls);assert.equal(body.children.length,1);
  const tools=body.children[0],status=tools.children[1],speed=tools.children[0].children[0];
  speed.value='1.5';speed.listeners.change();assert.equal(media.playbackRate,1.5);
  media.error={code:4};media.listeners.error();assert(status.textContent.length>15);
  assert.equal(/[\u0600-\u06ff]/.test(status.textContent),lang==='ar');
  media.listeners.playing();assert.equal(status.textContent,'');
  vm.runInNewContext(script,{document});assert.equal(body.children.length,1,'no duplicate controls');
}
assert.equal(player('en','text/html').body.children.length,0,'leave site players untouched');
console.log('Media controls, errors, localization and no-autoplay checks passed');
