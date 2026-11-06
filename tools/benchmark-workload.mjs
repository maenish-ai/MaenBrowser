// Actual Windows CEF synthetic-workload runner. No account data or real-site claim.
import fs from 'node:fs/promises';
import path from 'node:path';
import os from 'node:os';
import {spawn} from 'node:child_process';
import {createServer} from 'node:http';
import {createServer as createTcpServer} from 'node:net';
import {fileURLToPath} from 'node:url';
const root=path.resolve(path.dirname(fileURLToPath(import.meta.url)),'..');
const pause=ms=>new Promise(r=>setTimeout(r,ms));
async function freePort(){const s=createTcpServer();await new Promise(r=>s.listen(0,'127.0.0.1',r));const port=s.address().port;await new Promise(r=>s.close(r));return port;}
async function command(exe,args,options={}){
 const child=spawn(exe,args,{stdio:'inherit',...options});
 await new Promise((resolve,reject)=>{child.on('error',reject);child.on('exit',code=>code===0?resolve():reject(Error('Command failed: '+exe+' ('+code+')')));});
}
function rpc(socket){let id=0;const pending=new Map();
 socket.addEventListener('message',event=>{const m=JSON.parse(event.data);const p=pending.get(m.id);if(p){clearTimeout(p.timer);pending.delete(m.id);m.error?p.reject(Error(m.error.message)):p.resolve(m.result);}});
 socket.addEventListener('close',()=>{for(const p of pending.values()){clearTimeout(p.timer);p.reject(Error('CDP connection closed'));}pending.clear();});
 return (method,params={})=>new Promise((resolve,reject)=>{const n=++id;const timer=setTimeout(()=>{pending.delete(n);reject(Error('CDP timeout: '+method));},15000);pending.set(n,{resolve,reject,timer});socket.send(JSON.stringify({id:n,method,params}));});
}
async function main(){
 if(process.platform!=='win32')throw Error('Run on a real Windows device. No native benchmark is fabricated on other systems.');
 const [executable,output,mode='auto',durationText='60']=process.argv.slice(2);
 if(!executable||!output||!['auto','ultra','lite','balanced','performance'].includes(mode))throw Error('Usage: node tools/benchmark-workload.mjs <MaenBrowser.exe> <output-directory> [auto|ultra|lite|balanced|performance] [seconds]');
 const duration=Number(durationText);if(!Number.isInteger(duration)||duration<10||duration>3600)throw Error('Duration must be10–3600seconds');
 const exe=path.resolve(executable),out=path.resolve(output);await fs.access(exe);await fs.mkdir(out,{recursive:true});
 const fixture=createServer((_req,res)=>{res.writeHead(200,{'Content-Type':'text/html; charset=utf-8','Cache-Control':'no-store'});res.end('<!doctype html><meta charset="utf-8"><title>MaenBrowser local benchmark</title><h1>Local text and draft fixture</h1><textarea aria-label="Unsaved draft">Keep this draft</textarea><p>Static local workload. This is not WhatsApp, a call, or a video benchmark.</p>');});
 await new Promise(r=>fixture.listen(0,'127.0.0.1',r));const origin='http://127.0.0.1:'+fixture.address().port;
 try{for(const count of [1,5,10,20]){
  const profile=await fs.mkdtemp(path.join(os.tmpdir(),'maen-perf-'));await fs.writeFile(path.join(profile,'resource-mode.txt'),mode+'\n');
  const port=await freePort(),started=performance.now();let socket;
  const browser=spawn(exe,['--remote-debugging-port='+port,'--remote-debugging-address=127.0.0.1','--no-first-run','--no-default-browser-check'],{cwd:path.dirname(exe),stdio:'ignore',env:{...process.env,MAEN_BROWSER_TEST_PROFILE:profile}});
  browser.on('error',()=>{});
  try{
   let endpoint;
   for(let i=0;i<120;i++){
    if(browser.exitCode!==null)throw Error('Browser exited before benchmark startup');
    try{const r=await fetch(`http://127.0.0.1:${port}/json/version`,{signal:AbortSignal.timeout(1000)});endpoint=(await r.json()).webSocketDebuggerUrl;if(endpoint)break;}catch{}
    await pause(500);
   }
   if(!endpoint)throw Error('Browser debug endpoint did not become ready');
   const ready=performance.now()-started;socket=new WebSocket(endpoint);await new Promise((resolve,reject)=>{socket.onopen=resolve;socket.onerror=reject;});const send=rpc(socket);
   const info=await send('SystemInfo.getProcessInfo');
   if(!info.processInfo.some(p=>p.type==='browser'&&p.id===browser.pid))throw Error('Debug endpoint does not belong to the launched test process');
   const before=(await send('Target.getTargets')).targetInfos.filter(t=>t.type==='page');
   for(let i=0;i<count;i++)await send('Target.createTarget',{url:origin+'/fixture?tab='+i});
   for(const target of before)await send('Target.closeTarget',{targetId:target.targetId});
   await pause(10000);
   const targets=(await send('Target.getTargets')).targetInfos;
   const actual=targets.filter(t=>t.type==='page'&&t.url.startsWith(origin+'/fixture?tab=')).length;
   if(actual!==count)throw Error('Verified fixture tab count differs from requested count');
   const capture=path.join(out,`${mode}-${count}-tabs.json`);
   await command('pwsh',['-NoProfile','-File',path.join(root,'tools/measure-browser-processes.ps1'),'-ExecutablePath',exe,'-RootProcessId',String(browser.pid),'-OutputPath',capture,'-DeclaredTabs',String(count),'-Scenario','idle','-DurationSeconds',String(duration)]);
   const after=(await send('Target.getTargets')).targetInfos.filter(t=>t.type==='page'&&t.url.startsWith(origin+'/fixture?tab=')).length;
   await fs.writeFile(path.join(out,`${mode}-${count}-workload.json`),JSON.stringify({schema:1,mode,requestedTabs:count,verifiedTabsBefore:actual,verifiedTabsAfter:after,debugEndpointReadyMs:ready,settlingSeconds:10,workload:'Same-origin local static text plus unsaved textarea; synthetic, no multi-site isolation/account/call/video performance claim',browserVersion:await send('Browser.getVersion'),capture:path.basename(capture)},null,2));
   await send('Browser.close').catch(()=>{});
  }finally{
   socket?.close();
   if(browser.exitCode===null)await command('taskkill',['/PID',String(browser.pid),'/T','/F']).catch(()=>{});
   // Only this runner's unique temporary profile is removed.
   await fs.rm(profile,{recursive:true,force:true,maxRetries:10,retryDelay:500}).catch(()=>{});
  }
 }}finally{await new Promise(r=>fixture.close(r));}
 console.log('Actual all-process captures saved. Repeat on4/8/16GB devices and record OS/CPU/GPU/workload context; do not equate this fixture with real-site performance.');
}
main().catch(error=>{console.error(error.message);process.exitCode=1;});
