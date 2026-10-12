// Windows CI: exercise the actual dedicated STA window, HTTP redirect,
// H.264 decoded frames, AAC capability and the production resource filter.
import {spawn, execFileSync} from 'node:child_process';
import {createServer} from 'node:http';
import fs from 'node:fs';
import path from 'node:path';
import os from 'node:os';
import {randomUUID} from 'node:crypto';
import assert from 'node:assert/strict';

const exe = path.resolve(process.argv[2]);
const token = randomUUID();
const clip = fs.readFileSync(new URL('./fixtures/h264-aac.mp4', import.meta.url));
const profile = fs.mkdtempSync(path.join(os.tmpdir(), 'maen-media-test-'));
let resolveResult;
const result = new Promise(resolve => { resolveResult = resolve; });
const page = `<!doctype html><meta charset="utf-8"><title>Maen media playback test</title>
<video id="v" controls muted autoplay playsinline src="/clip.mp4"></video>
<script>
const v=document.getElementById('v');
const capability={h264:v.canPlayType('video/mp4; codecs="avc1.42E01E"'),aac:v.canPlayType('audio/mp4; codecs="mp4a.40.2"')};
let sent=false;
function report(value){if(sent)return;sent=true;fetch('/result/${token}',{method:'POST',body:JSON.stringify({...capability,...value})});}
v.addEventListener('error',()=>report({error:v.error?.code}));
v.addEventListener('timeupdate',()=>{if(v.currentTime>1)report({time:v.currentTime,width:v.videoWidth,frames:v.getVideoPlaybackQuality().totalVideoFrames});});
v.play().catch(e=>report({error:e.name}));
setTimeout(()=>report({error:'playback timeout',state:v.readyState}),20000);
</script>`;
const server = createServer((req,res) => {
  if (req.url === '/redirect') { res.writeHead(302, {Location:'/fixture'}); res.end(); return; }
  if (req.url === '/fixture') { res.writeHead(200, {'Content-Type':'text/html'}); res.end(page); return; }
  if (req.url === `/result/${token}` && req.method === 'POST') {
    let body=''; req.on('data', chunk=>{body+=chunk;if(body.length>4096)req.destroy();});
    req.on('end',()=>{try{resolveResult(JSON.parse(body));}catch{resolveResult({error:'invalid result'});}res.end('ok');}); return;
  }
  if (req.url === '/clip.mp4') {
    const range = /^bytes=(\d+)-(\d*)$/.exec(req.headers.range || '');
    if (range) {
      const start=Number(range[1]); const end=Math.min(range[2]?Number(range[2]):clip.length-1,clip.length-1);
      if(start>end){res.writeHead(416);res.end();return;}
      res.writeHead(206, {'Content-Type':'video/mp4','Accept-Ranges':'bytes','Content-Range':`bytes ${start}-${end}/${clip.length}`,'Content-Length':end-start+1});
      res.end(clip.subarray(start,end+1)); return;
    }
    res.writeHead(200, {'Content-Type':'video/mp4','Accept-Ranges':'bytes','Content-Length':clip.length});res.end(clip);return;
  }
  res.writeHead(404);res.end();
});
await new Promise(resolve => server.listen(0,'127.0.0.1',resolve));
let child, timer;
try {
  const url=`http://127.0.0.1:${server.address().port}/redirect`;
  child=spawn(exe,[`--maen-media-url=${url}`,'--no-first-run','--no-default-browser-check'],{
    cwd:path.dirname(exe),stdio:'inherit',env:{...process.env,LOCALAPPDATA:profile}
  });
  const failure=new Promise((_,reject)=>{
    child.once('error',reject);
    child.once('exit',code=>reject(new Error(`Browser exited before playback result: ${code}`)));
    timer=setTimeout(()=>reject(new Error('WebView2 media test timed out; check runtime installation and MB-704')),75000);
  });
  const playback=await Promise.race([result,failure]);
  assert(!playback.error,JSON.stringify(playback));
  assert(playback.h264 && playback.aac,'H.264/AAC capability missing');
  assert(playback.time>1 && playback.width===160 && playback.frames>0,JSON.stringify(playback));
  console.log('PASS: real WebView2 window, redirect and H.264 decoded frames; AAC advertised.',JSON.stringify(playback));
  console.log('This muted automation does not verify audible sound or every website.');
} finally {
  clearTimeout(timer);
  if(child?.pid)try{execFileSync('taskkill',['/PID',String(child.pid),'/T','/F'],{stdio:'ignore'});}catch{}
  server.closeAllConnections();server.close();
  fs.rmSync(profile,{recursive:true,force:true,maxRetries:8,retryDelay:250});
}
