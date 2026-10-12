// Real CEF integration on the Windows CI runner. The debug port exists only
// for this short-lived test process and is never enabled by a release default.
import {spawn,execFileSync} from 'node:child_process';
import fs from 'node:fs';
import path from 'node:path';
import os from 'node:os';
import {createServer} from 'node:http';
import {strict as assert} from 'node:assert';
import {revealVideoControls} from '../assets/companion/video-controls.js';
const exe=path.resolve(process.argv[2]);
const formats=JSON.parse(fs.readFileSync(new URL('./fixtures/formats.json',import.meta.url)));
const formatBytes=new Map(formats.map(f=>[f.file,fs.readFileSync(new URL('./fixtures/'+f.file,import.meta.url))]));
const h264Fixture=fs.readFileSync(new URL('./fixtures/h264-aac.mp4',import.meta.url));
const id=fs.readFileSync('src/protection/identity.h','utf8').match(/kExtensionId\[\] = "([a-p]+)"/)[1];
const testProfile=fs.mkdtempSync(path.join(os.tmpdir(),'maen-integration-'));
const launch=()=>spawn(exe,['--remote-debugging-port=9222','--no-first-run','--no-default-browser-check'],{cwd:path.dirname(exe),stdio:'ignore',env:{...process.env,MAEN_BROWSER_TEST_PROFILE:testProfile}});
let processHandle=launch();
const pause=ms=>new Promise(r=>setTimeout(r,ms));
let fixture;
let ws;let serial=0;const pending=new Map();
function send(method,params={}){return new Promise((resolve,reject)=>{const n=++serial;const timer=setTimeout(()=>{pending.delete(n);reject(new Error('CDP timeout: '+method));},20000);pending.set(n,{resolve:v=>{clearTimeout(timer);resolve(v);},reject:e=>{clearTimeout(timer);reject(e);}});ws.send(JSON.stringify({id:n,method,params}));});}
async function evaluate(expression){const r=await send('Runtime.evaluate',{expression,awaitPromise:true,returnByValue:true});if(r.exceptionDetails)throw new Error(JSON.stringify(r.exceptionDetails));return r.result.value;}
async function navigate(url) {
  const result=await send('Page.navigate',{url});
  assert(!result.errorText, 'Navigation failed: '+result.errorText);
  for(let n=0;n<100;n++) {
    try {
      if(await evaluate(`location.href===${JSON.stringify(url)} && document.readyState==='complete'`)) return;
    } catch {} // Execution contexts are replaced during navigation.
    await pause(100);
  }
  throw new Error('Page did not finish loading: '+url);
}
const rawCall=object=>evaluate(`(async()=>{const r=await fetch('https://maen.browser/api',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(${JSON.stringify(object)})});return {status:r.status,data:await r.json()};})()`);
async function call(object) {
  // Test convenience only. Real panels use the revision they displayed.
  if(object.op==='save'&&!Object.hasOwn(object,'expectedRevision')) {
    const current=await rawCall({op:'get'});object={...object,expectedRevision:current.data.revision};
  }
  return rawCall(object);
}
async function connectBrowser(){
  let target;
  for(let n=0;n<60;n++){
    try{const targets=await(await fetch('http://127.0.0.1:9222/json/list')).json();target=targets.find(x=>x.type==='page'&&x.url.startsWith('chrome-extension://'+id+'/'));if(target)break;}catch{}
    await pause(1000);
  }
  assert(target,'Bundled extension/start page did not load in real CEF');
  ws=new WebSocket(target.webSocketDebuggerUrl);
  await new Promise((resolve,reject)=>{ws.onopen=resolve;ws.onerror=reject;});
  ws.onmessage=e=>{const m=JSON.parse(e.data);if(m.id&&pending.has(m.id)){const p=pending.get(m.id);pending.delete(m.id);m.error?p.reject(new Error(m.error.message)):p.resolve(m.result);}};
  return target;
}
try{
  const target=await connectBrowser();
  const technical=await call({op:'technical'});
  assert.equal(technical.status,200);assert.equal(technical.data.ok,true);
  assert(technical.data.engine.startsWith('CEF '));
  assert(['UltraLite','Lite','Balanced','Performance'].includes(technical.data.resourceMode));
  assert(technical.data.browserProcessMemoryMiB>0);
  assert(technical.data.browserProcessCpuSeconds>=0);
  let result=await call({op:'get'});assert.equal(result.status,200);assert.equal(result.data.ok,true);assert(result.data.adDomains>1000);assert.equal(result.data.familyListsLoaded,false);assert.equal(result.data.adultDomains,0);
  console.log('PASS: real CEF extension loaded and native API connected');
  result=await call({op:'save',settings:{ads:false,askDownload:false}});assert(result.data.ok);assert.equal(result.data.ads,false);assert.equal(result.data.askDownload,false);
  result=await call({op:'save',settings:{ads:true,askDownload:true}});assert(result.data.ok);
  const staleRevision=result.data.revision;
  result=await call({op:'save',settings:{askDownload:false}});assert(result.data.ok);
  const conflict=await rawCall({op:'save',expectedRevision:staleRevision,settings:{ads:false}});
  assert.equal(conflict.data.ok,false);assert.match(conflict.data.error,/another window/);
  assert.equal((await call({op:'get'})).data.ads,true);
  assert.equal((await rawCall({op:'save',settings:{ads:false}})).data.ok,false);
  assert.equal(await evaluate(`document.getElementById('adToggle')===null`),true);
  console.log('PASS: stale and revisionless saves rejected; clean search homepage');
  // The extension intentionally allows fetch only to maen.browser via CSP.
  // Exercise ad interception from an ordinary page, not a privileged extension
  // document whose CSP would reject this request before native filtering.
  fixture=createServer((request,response)=>{
    if(request.url.startsWith('/format/')) {
      const name=request.url.slice('/format/'.length),format=formats.find(f=>f.file===name);
      if(!format){response.writeHead(404);response.end();return;}
      const bytes=formatBytes.get(name);
      response.writeHead(200,{'Content-Type':format.mime,'Content-Length':bytes.length,'Cache-Control':'no-store'});
      response.end(bytes);return;
    }
    if(request.url==='/media'){
      response.writeHead(200,{'Content-Type':'text/html; charset=utf-8','Cache-Control':'no-store'});
      response.end('<!doctype html><meta charset="utf-8"><title>MaenBrowser H264 test</title><video id="h264" controls muted autoplay playsinline src="/clip.mp4"></video>');
      return;
    }
    if(request.url==='/clip.mp4'){
      const range=/^bytes=(\d+)-(\d*)$/.exec(request.headers.range||'');
      if(range){
        const start=Number(range[1]),end=Math.min(range[2]?Number(range[2]):h264Fixture.length-1,h264Fixture.length-1);
        if(start>end){response.writeHead(416);response.end();return;}
        response.writeHead(206,{'Content-Type':'video/mp4','Accept-Ranges':'bytes','Content-Range':`bytes ${start}-${end}/${h264Fixture.length}`,'Content-Length':end-start+1,'Cache-Control':'no-store'});
        response.end(h264Fixture.subarray(start,end+1));return;
      }
      response.writeHead(200,{'Content-Type':'video/mp4','Accept-Ranges':'bytes','Content-Length':h264Fixture.length,'Cache-Control':'no-store'});
      response.end(h264Fixture);return;
    }
    if(request.url==='/download'){
      const chunk=Buffer.alloc(65536,0x6d);let count=0;
      response.writeHead(200,{'Content-Type':'text/plain','Content-Disposition':'attachment; filename="maen-download-test.txt"','Content-Length':String(chunk.length*32),'Cache-Control':'no-store'});
      const timer=setInterval(()=>{response.write(chunk);if(++count===32){clearInterval(timer);response.end();}},100);
      response.on('close',()=>clearInterval(timer));return;
    }
    response.writeHead(200,{'Content-Type':'text/html; charset=utf-8','Cache-Control':'no-store'});
    response.end('<!doctype html><meta charset="utf-8"><title>MaenBrowser request test</title>');
  });
  await new Promise((resolve,reject)=>{fixture.once('error',reject);fixture.listen(0,'127.0.0.1',resolve);});
  const fixtureUrl=`http://127.0.0.1:${fixture.address().port}/`;
  const controlUrl=target.url;
  const before=Number(result.data.blockedRequests);
  await navigate(fixtureUrl);
  const controls = await evaluate(`(() => {
    const video=document.createElement('video'); document.body.append(video);
    const count=(${revealVideoControls.toString()})();
    return {count,controls:video.controls,paused:video.paused};
  })()`);
  assert.equal(controls.count,1); assert(controls.controls); assert(controls.paused);
  console.log('PASS: HTML5 controls enabled in a real CEF webpage without autoplay');
  // Exercise H.264/AAC in the actual CEF tab: no WebView2 overlay, popup, or
  // separate media session. This catches a stock CEF runtime lacking codecs.
  await navigate(fixtureUrl+'media');
  await evaluate(`document.getElementById('h264').play().catch(()=>{})`);
  let mediaState;
  for(let n=0;n<60;n++){
    mediaState=await evaluate(`(()=>{const v=document.getElementById('h264');return {h264:v.canPlayType('video/mp4; codecs="avc1.42E01E"'),aac:v.canPlayType('audio/mp4; codecs="mp4a.40.2"'),time:v.currentTime,width:v.videoWidth,frames:v.getVideoPlaybackQuality().totalVideoFrames,error:v.error?.code||0}})()`);
    if(mediaState.error||mediaState.time>1&&mediaState.width===160&&mediaState.frames>0)break;
    await pause(250);
  }
  assert(mediaState.h264&&mediaState.aac,'CEF runtime lacks H.264/AAC support: '+JSON.stringify(mediaState));
  assert(mediaState.time>1&&mediaState.width===160&&mediaState.frames>0,'CEF did not decode H.264 frames in the active tab: '+JSON.stringify(mediaState));
  console.log('PASS: H.264/AAC decoded and played in the same CEF tab with native controls',JSON.stringify(mediaState));
  // Also exercise Chromium's direct MP4 document, not only an embedded player.
  await navigate(fixtureUrl+'clip.mp4');
  let directState;
  for(let n=0;n<60;n++){
    directState=await evaluate(`(()=>{const v=document.querySelector('video');return v?{controls:v.controls,width:v.videoWidth,time:v.currentTime,error:v.error?.code||0}:null})()`);
    if(directState?.error||directState?.time>1&&directState?.width===160)break;
    await pause(250);
  }
  assert(directState?.controls&&directState.time>1&&directState.width===160,'Direct MP4 did not play in the active CEF tab: '+JSON.stringify(directState));
  console.log('PASS: direct MP4 document plays in the same CEF tab');
  for(const format of formats) {
    await navigate(fixtureUrl);
    await evaluate(`(()=>{const p=document.createElement('video');p.id='codec';p.muted=true;p.controls=true;p.src=${JSON.stringify(fixtureUrl)}+'format/'+${JSON.stringify(format.file)};document.body.append(p);p.play().catch(()=>{});})()`);
    let state;
    for(let n=0;n<60;n++) {
      state=await evaluate(`(()=>{const p=document.getElementById('codec');return {time:p.currentTime,width:p.videoWidth,error:p.error?.code||0};})()`);
      if(state.error||state.time>0.5)break;
      await pause(100);
    }
    assert(!state.error&&state.time>0.5&&(!format.video||state.width===160),'Codec playback failed '+format.file+': '+JSON.stringify(state));
    console.log('PASS: actual codec decode/time advancement',format.file);
  }
  await navigate(fixtureUrl);
  const probe=await evaluate(`fetch('https://doubleclick.net/maen-smoke?run='+Date.now(), {
    mode:'no-cors', cache:'no-store', signal:AbortSignal.timeout(5000)
  }).then(r=>({resolved:true,status:r.status})).catch(e=>({resolved:false,error:e.name,message:e.message}))`);
  await navigate(controlUrl);
  result=await call({op:'get'});
  const after=Number(result.data.blockedRequests);
  console.log('Native ad probe:',JSON.stringify({before,after,probe}));
  assert(after>before,'Native resource blocking did not run from the ordinary test page');
  assert.equal(probe.resolved,false,'Blocked request unexpectedly completed');
  console.log('PASS: native ad toggle, download preference and request interception');
  result=await call({op:'save',settings:{family:true,allowOnly:true,allowed:['example.com']},newPin:'smoke-test-730192'});assert(result.data.ok);
  assert(result.data.familyListsLoaded);assert(result.data.adultDomains>1000);
  result=await call({op:'save',settings:{family:false},pin:'wrong'});assert.equal(result.data.ok,false);
  await pause(5100);
  result=await call({op:'save',settings:{family:false},pin:'smoke-test-730192'});assert(result.data.ok);assert.equal(result.data.family,false);assert.equal(result.data.familyListsLoaded,false);
  console.log('PASS: protected family settings and wrong-PIN rejection');
  const settingsFile=path.join(testProfile,'controls.dat');const bytes=fs.readFileSync(settingsFile);assert(!bytes.includes(Buffer.from('smoke-test-730192')));assert(!bytes.includes(Buffer.from('pinHash')));
  console.log('PASS: settings persisted as Windows DPAPI-protected bytes');
  const service=await evaluate(`chrome.runtime.sendMessage({op:'settingsChanged',wasFamily:false})`);assert.equal(service.ok,true,service.error);
  console.log('PASS: companion service worker/native API integration');
  // Open the toolbar over a normal web tab, matching the reported scenario.
  const normalTab=await evaluate(`chrome.tabs.create({url:${JSON.stringify(fixtureUrl)},active:true})`);
  await pause(400);
  await evaluate(`(async()=>{const w=await chrome.windows.getCurrent();await chrome.windows.update(w.id,{focused:true});await chrome.action.openPopup();})()`);
  let popupTarget;
  for(let n=0;n<40;n++){
    const all=await(await fetch('http://127.0.0.1:9222/json/list')).json();
    popupTarget=all.find(x=>x.url===`chrome-extension://${id}/popup.html`);
    if(popupTarget)break;await pause(250);
  }
  assert(popupTarget,'Toolbar action popup was not exposed for integration testing');
  const popupSocket=new WebSocket(popupTarget.webSocketDebuggerUrl);
  try{
    await new Promise((resolve,reject)=>{popupSocket.onopen=resolve;popupSocket.onerror=reject;});
    const popupResult=await new Promise((resolve,reject)=>{
      const timeout=setTimeout(()=>reject(new Error('Action popup API timeout')),20000);
      popupSocket.onmessage=event=>{const msg=JSON.parse(event.data);if(msg.id===1){clearTimeout(timeout);msg.error?reject(new Error(msg.error.message)):resolve(msg.result);}};
      popupSocket.send(JSON.stringify({id:1,method:'Runtime.evaluate',params:{awaitPromise:true,returnByValue:true,expression:`(async()=>{
        const {api}=await import(chrome.runtime.getURL('api.js'));
        const result=await api();
        for(let n=0;n<30 && document.getElementById('toggle').disabled;n++)await new Promise(r=>setTimeout(r,100));
        return {ok:result.ok,ads:result.ads,ready:!document.getElementById('toggle').disabled,state:document.getElementById('state').textContent};
      })()`}}));
    });
    assert(!popupResult.exceptionDetails,JSON.stringify(popupResult.exceptionDetails));
    assert.equal(popupResult.result.value.ok,true);
    assert.equal(popupResult.result.value.ads,true);
    assert.equal(popupResult.result.value.ready,true);
    assert(['ON','مفعّل'].includes(popupResult.result.value.state),'Popup is showing a disconnected state');
  }finally{popupSocket.close();}
  await evaluate(`chrome.tabs.remove(${normalTab.id})`);
  console.log('PASS: actual toolbar popup over a normal web tab connected to native protection');
  await evaluate(`chrome.runtime.sendMessage({op:'nativeApi',operation:'setLanguage',extra:{language:'ar'}})`);
  await evaluate(`chrome.storage.local.set({language:'ar'})`);
  await pause(300);
  assert.equal(await evaluate(`document.documentElement.lang`),'ar');
  assert.equal(await evaluate(`document.documentElement.dir`),'rtl');
  await evaluate(`chrome.runtime.sendMessage({op:'nativeApi',operation:'setLanguage',extra:{language:'en'}})`);
  await evaluate(`chrome.storage.local.set({language:'en'})`);
  await pause(300);
  assert.equal(await evaluate(`document.documentElement.lang`),'en');
  assert.equal(await evaluate(`document.documentElement.dir`),'ltr');
  console.log('PASS: live Arabic/English UI language and direction');

  // Exercise actual settings controls, not just their native API. Old profiles
  // can contain malformed optional preferences after upgrades.
  await evaluate(`chrome.storage.local.set({keepAwake:'legacy-invalid-value'})`);
  await navigate(`chrome-extension://${id}/options.html`);
  for(let n=0;n<100;n++){
    if(await evaluate(`!document.getElementById('fields').disabled`))break;
    await pause(100);
  }
  assert.equal(await evaluate(`document.getElementById('fields').disabled`),false,'Settings remained disabled');
  await evaluate(`document.getElementById('ads').click();document.getElementById('save').click()`);
  for(let n=0;n<100;n++){
    if(await evaluate(`!document.getElementById('save').disabled`))break;
    await pause(100);
  }
  assert.equal(await evaluate(`import('./api.js').then(m=>m.api()).then(s=>s.ads)`),false,
    'Checkbox save did not reach native protection: '+await evaluate(`document.getElementById('status').textContent`));
  await evaluate(`document.getElementById('ads').click();document.getElementById('save').click()`);
  for(let n=0;n<100;n++){
    if(await evaluate(`!document.getElementById('save').disabled`))break;
    await pause(100);
  }
  assert.equal(await evaluate(`import('./api.js').then(m=>m.api()).then(s=>s.ads)`),true);
  await evaluate(`chrome.storage.local.set({keepAwake:[]})`);
  await navigate(controlUrl);
  console.log('PASS: real settings checkboxes, save and malformed upgraded preferences');

  // A real loopback download verifies continuation, progress UI and that a
  // completion dialog no longer stalls the browser's UI thread.
  result=await call({op:'save',settings:{askDownload:false}});assert(result.data.ok);
  const downloadId=await evaluate(`chrome.downloads.download({url:${JSON.stringify(fixtureUrl+'download')},saveAs:false})`);
  await navigate(`chrome-extension://${id}/downloads.html`);
  let download,displayedRate=false;
  for(let n=0;n<100;n++){
    download=await evaluate(`chrome.downloads.search({id:${downloadId}}).then(items=>items[0])`);
    const details=await evaluate(`document.getElementById('items').textContent`);
    if(/(?:KiB|MiB|GiB)\/s/.test(details))displayedRate=true;
    if(download?.state==='complete')break;
    assert(download?.state!=='interrupted','Fixture download interrupted');
    await pause(150);
  }
  assert.equal(download?.state,'complete');assert.equal(download.bytesReceived,2097152);
  assert(displayedRate,'Visible downloads page never displayed a sampled speed');
  assert.equal(fs.statSync(download.filename).size,2097152);
  fs.unlinkSync(download.filename);
  await navigate(controlUrl);
  result=await call({op:'save',settings:{askDownload:true}});assert(result.data.ok);
  console.log('PASS: real download, visible speed, completion and responsive UI');

  await navigate(fixtureUrl);
  const untrusted=await evaluate(`fetch('https://maen.browser/api',{method:'POST',headers:{'Content-Type':'application/json'},body:'{"op":"get"}'}).then(r=>r.status).catch(()=>0)`);
  assert([0,403,503].includes(untrusted),'Web content accessed privileged settings');
  const untrustedWrite=await evaluate(`fetch('https://maen.browser/api',{method:'POST',headers:{'Content-Type':'application/json'},body:'{"op":"save","settings":{"ads":false}}'}).then(r=>r.status).catch(()=>0)`);
  assert([0,403,503].includes(untrustedWrite));
  await navigate(controlUrl);assert.equal((await call({op:'get'})).data.ads,true);
  console.log('PASS: untrusted web page cannot read or write native settings');
  // Persisted ad state must survive a real process restart.
  result=await call({op:'save',settings:{ads:false}});assert(result.data.ok);
  const exited=new Promise(resolve=>processHandle.once('exit',resolve));
  await send('Browser.close');
  await Promise.race([exited,pause(10000).then(()=>{throw new Error('Browser did not exit for persistence test');})]);
  ws.close();processHandle=launch();await connectBrowser();
  const persisted=await call({op:'get'});
  assert.equal(persisted.data.ads,false);assert.equal(persisted.data.family,false);
  assert.equal(persisted.data.revision,result.data.revision);
  console.log('PASS: encrypted ad preference survives real browser restart');
  console.log('MaenBrowser Windows integration smoke passed');
}finally{
  if(ws)ws.close();
  if(fixture){fixture.closeAllConnections();fixture.close();}
  if(process.platform==='win32'){try{execFileSync('taskkill',['/PID',String(processHandle.pid),'/T','/F'],{stdio:'ignore'});}catch{}}
  else processHandle.kill();
  try{fs.rmSync(testProfile,{recursive:true,force:true,maxRetries:10,retryDelay:100});}catch{}
}
