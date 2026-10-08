// Real CEF integration on the Windows CI runner. The debug port exists only
// for this short-lived test process and is never enabled by a release default.
import {spawn,execFileSync} from 'node:child_process';
import fs from 'node:fs';
import path from 'node:path';
import {createServer} from 'node:http';
import {strict as assert} from 'node:assert';
const exe=path.resolve(process.argv[2]);
const id=fs.readFileSync('src/protection/identity.h','utf8').match(/kExtensionId\[\] = "([a-p]+)"/)[1];
const processHandle=spawn(exe,['--remote-debugging-port=9222','--no-first-run','--no-default-browser-check'],{cwd:path.dirname(exe),stdio:'ignore'});
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
const call=object=>evaluate(`(async()=>{const r=await fetch('https://maen.browser/api',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(${JSON.stringify(object)})});return {status:r.status,data:await r.json()};})()`);
try{
  let target;
  for(let n=0;n<60;n++){
    try{const targets=await(await fetch('http://127.0.0.1:9222/json/list')).json();target=targets.find(x=>x.type==='page'&&x.url.startsWith('chrome-extension://'+id+'/'));if(target)break;}catch{}
    await pause(1000);
  }
  assert(target,'Bundled extension/start page did not load in real CEF');
  ws=new WebSocket(target.webSocketDebuggerUrl);
  await new Promise((resolve,reject)=>{ws.onopen=resolve;ws.onerror=reject;});
  ws.onmessage=e=>{const m=JSON.parse(e.data);if(m.id&&pending.has(m.id)){const p=pending.get(m.id);pending.delete(m.id);m.error?p.reject(new Error(m.error.message)):p.resolve(m.result);}};
  let result=await call({op:'get'});assert.equal(result.status,200);assert.equal(result.data.ok,true);assert(result.data.adDomains>1000);assert(result.data.adultDomains>1000);
  console.log('PASS: real CEF extension loaded and native API connected');
  result=await call({op:'save',settings:{ads:false,askDownload:false}});assert(result.data.ok);assert.equal(result.data.ads,false);assert.equal(result.data.askDownload,false);
  result=await call({op:'save',settings:{ads:true,askDownload:true}});assert(result.data.ok);
  // The extension intentionally allows fetch only to maen.browser via CSP.
  // Exercise ad interception from an ordinary page, not a privileged extension
  // document whose CSP would reject this request before native filtering.
  fixture=createServer((request,response)=>{
    response.writeHead(200,{'Content-Type':'text/html; charset=utf-8','Cache-Control':'no-store'});
    response.end('<!doctype html><meta charset="utf-8"><title>MaenBrowser request test</title>');
  });
  await new Promise((resolve,reject)=>{fixture.once('error',reject);fixture.listen(0,'127.0.0.1',resolve);});
  const fixtureUrl=`http://127.0.0.1:${fixture.address().port}/`;
  const controlUrl=target.url;
  const before=Number(result.data.blockedRequests);
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
  result=await call({op:'save',settings:{family:false},pin:'wrong'});assert.equal(result.data.ok,false);
  await pause(5100);
  result=await call({op:'save',settings:{family:false},pin:'smoke-test-730192'});assert(result.data.ok);assert.equal(result.data.family,false);
  console.log('PASS: protected family settings and wrong-PIN rejection');
  const settingsFile=path.join(process.env.LOCALAPPDATA,'MaenBrowser','controls.dat');const bytes=fs.readFileSync(settingsFile);assert(!bytes.includes(Buffer.from('smoke-test-730192')));assert(!bytes.includes(Buffer.from('pinHash')));
  console.log('PASS: settings persisted as Windows DPAPI-protected bytes');
  const service=await evaluate(`chrome.runtime.sendMessage({op:'settingsChanged',wasFamily:false})`);assert.equal(service.ok,true,service.error);
  console.log('PASS: companion service worker/native API integration');
  await navigate(fixtureUrl);
  const untrusted=await evaluate(`fetch('https://maen.browser/api',{method:'POST',headers:{'Content-Type':'application/json'},body:'{"op":"get"}'}).then(r=>r.status).catch(()=>0)`);
  assert([0,403,503].includes(untrusted),'Web content accessed privileged settings');
  console.log('PASS: untrusted web page cannot access native settings');
  console.log('MaenBrowser Windows integration smoke passed');
}finally{
  if(ws)ws.close();
  if(fixture){fixture.closeAllConnections();fixture.close();}
  if(process.platform==='win32'){try{execFileSync('taskkill',['/IM','MaenBrowser.exe','/T','/F'],{stdio:'ignore'});}catch{}}
  else processHandle.kill();
}
