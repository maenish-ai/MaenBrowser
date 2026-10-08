// Real CEF integration on the Windows CI runner. The debug port exists only
// for this short-lived test process and is never enabled by a release default.
import {spawn,execFileSync} from 'node:child_process';
import fs from 'node:fs';
import path from 'node:path';
import {strict as assert} from 'node:assert';
const exe=path.resolve(process.argv[2]);
const id=fs.readFileSync('src/protection/identity.h','utf8').match(/kExtensionId\[\] = "([a-p]+)"/)[1];
const processHandle=spawn(exe,['--remote-debugging-port=9222','--no-first-run','--no-default-browser-check'],{cwd:path.dirname(exe),stdio:'ignore'});
const pause=ms=>new Promise(r=>setTimeout(r,ms));
let ws;let serial=0;const pending=new Map();
function send(method,params={}){return new Promise((resolve,reject)=>{const n=++serial;const timer=setTimeout(()=>{pending.delete(n);reject(new Error('CDP timeout: '+method));},20000);pending.set(n,{resolve:v=>{clearTimeout(timer);resolve(v);},reject:e=>{clearTimeout(timer);reject(e);}});ws.send(JSON.stringify({id:n,method,params}));});}
async function evaluate(expression){const r=await send('Runtime.evaluate',{expression,awaitPromise:true,returnByValue:true});if(r.exceptionDetails)throw new Error(JSON.stringify(r.exceptionDetails));return r.result.value;}
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
  const before=Number(result.data.blockedRequests);
  await evaluate(`fetch('https://doubleclick.net/maen-smoke').catch(()=>null)`);
  result=await call({op:'get'});assert(Number(result.data.blockedRequests)>before,'Native resource blocking did not run');
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
  await send('Page.navigate',{url:'data:text/html,<title>Untrusted test</title>'});await pause(1000);
  const untrusted=await evaluate(`fetch('https://maen.browser/api',{method:'POST',headers:{'Content-Type':'application/json'},body:'{"op":"get"}'}).then(r=>r.status).catch(()=>0)`);
  assert([0,403,503].includes(untrusted),'Web content accessed privileged settings');
  console.log('PASS: untrusted web page cannot access native settings');
  console.log('MaenBrowser Windows integration smoke passed');
}finally{
  if(ws)ws.close();
  if(process.platform==='win32'){try{execFileSync('taskkill',['/IM','MaenBrowser.exe','/T','/F'],{stdio:'ignore'});}catch{}}
  else processHandle.kill();
}
