
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
document.getElementById("mediaCheck").addEventListener("click",()=>{
 const v=document.createElement("video"),a=document.createElement("audio");
 const tests=[
  ["H.264",v.canPlayType('video/mp4; codecs="avc1.42E01E"')],
  ["HEVC/H.265",v.canPlayType('video/mp4; codecs="hvc1"')],
  ["VP9",v.canPlayType('video/webm; codecs="vp09.00.10.08"')],
  ["AV1",v.canPlayType('video/mp4; codecs="av01.0.05M.08"')],
  ["AAC",a.canPlayType('audio/mp4; codecs="mp4a.40.2"')],
  ["Opus",a.canPlayType('audio/webm; codecs="opus"')],
  ["FLAC",a.canPlayType('audio/flac')],
  ["MP3",a.canPlayType('audio/mpeg')]
 ];
 let webgl=false;try{webgl=!!document.createElement("canvas").getContext("webgl2")}catch(e){}
 const lines=tests.map(x=>x[0]+": "+(x[1]||"not reported"));
 lines.push("WebGL2: "+(webgl?"available":"unavailable"));
 lines.push("CPU threads reported: "+(navigator.hardwareConcurrency||"unknown"));
 const out=document.getElementById("mediaResult");out.textContent=lines.join("\n");out.style.display="block";
});
paint();

