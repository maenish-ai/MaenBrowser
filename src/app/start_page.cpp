#include "src/app/start_page.h"
#include <string>
#include "include/cef_parser.h"

namespace maenbrowser::ui {
std::string GetStartPageDataUrl() {
  static const char kHtml[] = R"HTML(<!doctype html><html><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1"><title>MaenBrowser</title>
<style>
:root{color-scheme:light dark;--accent:#3347df;--card:#fff;--line:#dfe4ee;--text:#172033;--muted:#697386}
*{box-sizing:border-box}body{margin:0;font-family:Segoe UI,Arial,sans-serif;background:#f5f7fb;color:var(--text);min-height:100vh}
main{width:min(880px,92vw);margin:auto;padding:8vh 0 36px}.brand{display:flex;justify-content:center;align-items:center;gap:12px;font-size:38px;font-weight:760}
.mark{display:grid;place-items:center;width:52px;height:52px;border-radius:15px;background:linear-gradient(135deg,#273ee8,#7955e8);color:#fff;font-weight:900}
.tag{text-align:center;color:var(--muted);margin:9px 0 25px}.search{display:flex;background:var(--card);border:1px solid var(--line);border-radius:17px;padding:7px;box-shadow:0 8px 28px #1c2b4a12}
.search input{flex:1;border:0;outline:0;background:transparent;color:inherit;padding:13px;font-size:16px}.search button{border:0;border-radius:11px;padding:0 22px;background:var(--accent);color:#fff;font-weight:650;cursor:pointer}
.title{margin:22px 2px 10px;font-size:13px;font-weight:700;color:var(--muted)}.engines{display:grid;grid-template-columns:repeat(4,1fr);gap:10px}
.engine{position:relative;border:1px solid var(--line);background:var(--card);border-radius:15px;padding:16px 10px;text-align:center;cursor:pointer;color:inherit}
.engine:hover{transform:translateY(-1px);box-shadow:0 7px 20px #24345410}.engine.active{outline:2px solid var(--accent);border-color:transparent}
.dot{width:34px;height:34px;border-radius:11px;margin:0 auto 8px;display:grid;place-items:center;background:#eef1f7;font-weight:800}.engine small{display:block;color:var(--muted);margin-top:4px}
.quick{display:grid;grid-template-columns:repeat(4,1fr);gap:10px}.quick a{background:var(--card);border:1px solid var(--line);border-radius:14px;padding:14px;text-align:center;text-decoration:none;color:inherit;font-weight:600}
.note{text-align:center;color:var(--muted);font-size:11px;margin-top:22px}
@media(max-width:650px){.engines,.quick{grid-template-columns:repeat(2,1fr)}.brand{font-size:31px}}
@media(prefers-color-scheme:dark){:root{--card:#1a2030;--line:#30394d;--text:#f4f6fb;--muted:#a8b0c1}body{background:#111522}.dot{background:#252d40}}
</style></head><body><main>
<div class="brand"><span class="mark">M</span>MaenBrowser</div><div class="tag">Choose your search. Keep your browser yours.</div>
<form id="search" class="search"><input id="q" autocomplete="off" autofocus placeholder="Search the web"><button>Search</button></form>
<div class="title">SEARCH ENGINE — choose anytime</div>
<div class="engines">
<button class="engine" data-engine="google"><span class="dot">G</span><b>Google</b><small>Search</small></button>
<button class="engine" data-engine="bing"><span class="dot">B</span><b>Bing</b><small>Search</small></button>
<button class="engine" data-engine="duckduckgo"><span class="dot">D</span><b>DuckDuckGo</b><small>Search</small></button>
<button class="engine" data-engine="brave"><span class="dot">Br</span><b>Brave Search</b><small>Search</small></button>
</div>
<div class="title">QUICK ACCESS</div>
<div class="quick"><a href="https://www.youtube.com/">YouTube</a><a href="https://mail.google.com/">Gmail</a><a href="https://web.whatsapp.com/">WhatsApp</a><a href="https://www.wikipedia.org/">Wikipedia</a></div>
<div class="note">Search choice is stored locally on this device. MaenBrowser is not affiliated with the listed services.</div>
</main><script>
const engines={
 google:q=>"https://www.google.com/search?q="+encodeURIComponent(q),
 bing:q=>"https://www.bing.com/search?q="+encodeURIComponent(q),
 duckduckgo:q=>"https://duckduckgo.com/?q="+encodeURIComponent(q),
 brave:q=>"https://search.brave.com/search?q="+encodeURIComponent(q)
};
let selected="google";
try{selected=localStorage.getItem("maen.searchEngine")||"google"}catch(e){}
if(!engines[selected])selected="google";
function paint(){document.querySelectorAll(".engine").forEach(x=>x.classList.toggle("active",x.dataset.engine===selected))}
document.querySelectorAll(".engine").forEach(x=>x.addEventListener("click",()=>{selected=x.dataset.engine;try{localStorage.setItem("maen.searchEngine",selected)}catch(e){}paint();document.getElementById("q").focus()}));
document.getElementById("search").addEventListener("submit",e=>{e.preventDefault();const q=document.getElementById("q").value.trim();if(q)location.href=engines[selected](q)});
paint();
</script></body></html>)HTML";
  return "data:text/html;charset=utf-8," + CefURIEncode(kHtml, false).ToString();
}

std::string MediaDiagnosticsUrl() {
  static const char kMediaHtml[] = R"HTML(<!doctype html>
<html><head><meta charset="utf-8"><meta name="viewport" content="width=device-width">
<title>MaenBrowser Media Diagnostics</title>
<style>
body{font-family:system-ui,Segoe UI,sans-serif;background:#0f1220;color:#eef1ff;margin:0;padding:28px}
main{max-width:920px;margin:auto}.card{background:#181d31;border:1px solid #2c3454;border-radius:16px;padding:18px;margin:14px 0}
h1{margin:0 0 6px}.ok{color:#76e6a5}.bad{color:#ff9a9a}.muted{color:#aeb8d8}
table{width:100%;border-collapse:collapse}td,th{text-align:left;padding:9px;border-bottom:1px solid #2c3454}
code{color:#c8d3ff}button{padding:9px 13px;border-radius:10px;border:0;cursor:pointer}
</style></head><body><main><h1>MaenBrowser Media Diagnostics</h1>
<p class="muted">Local capability test. No media or account data is uploaded.</p>
<div class="card"><h2>HTML5 codec claims</h2><table id="codecs"><tr><th>Format</th><th>Result</th></tr></table></div>
<div class="card"><h2>Browser media APIs</h2><table id="apis"></table></div>
<div class="card"><h2>Graphics</h2><pre id="gpu">Checking…</pre></div>
<div class="card"><button onclick="location.reload()">Run again</button></div>
<script>
const v=document.createElement('video'), a=document.createElement('audio');
const tests=[
['H.264 / MP4','video/mp4; codecs="avc1.42E01E"',v],
['AAC / MP4','audio/mp4; codecs="mp4a.40.2"',a],
['VP8 / WebM','video/webm; codecs="vp8"',v],
['VP9 / WebM','video/webm; codecs="vp09.00.10.08"',v],
['AV1','video/mp4; codecs="av01.0.05M.08"',v],
['Opus / WebM','audio/webm; codecs="opus"',a],
['Vorbis / WebM','audio/webm; codecs="vorbis"',a]
];
for(const [n,m,e] of tests){const r=e.canPlayType(m)||'no'; codecs.insertAdjacentHTML('beforeend',
`<tr><td>${n}<br><code>${m}</code></td><td class="${r==='no'?'bad':'ok'}">${r}</td></tr>`);}
const api=[['MediaSource',!!window.MediaSource],['Encrypted Media',!!navigator.requestMediaKeySystemAccess],
['WebRTC',!!window.RTCPeerConnection],['Picture-in-Picture',!!document.pictureInPictureEnabled],
['WebCodecs',!!window.VideoDecoder],['WebGL',(()=>{try{return !!document.createElement('canvas').getContext('webgl2')}catch(e){return false}})()]];
for(const [n,r] of api) apis.insertAdjacentHTML('beforeend',`<tr><td>${n}</td><td class="${r?'ok':'bad'}">${r?'Available':'Unavailable'}</td></tr>`);
try{const c=document.createElement('canvas'),g=c.getContext('webgl');const ext=g&&g.getExtension('WEBGL_debug_renderer_info');
gpu.textContent=g?('WebGL: available\\nRenderer: '+(ext?g.getParameter(ext.UNMASKED_RENDERER_WEBGL):'protected/unknown')):'WebGL: unavailable';}
catch(e){gpu.textContent='Graphics query failed: '+e;}
</script></main></body></html>)HTML";
  return std::string("data:text/html;charset=utf-8,") + CefURIEncode(kMediaHtml, false).ToString();
}

}  // namespace maenbrowser::ui
