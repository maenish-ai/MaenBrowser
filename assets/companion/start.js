import {t} from './i18n.js';

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
 const lines=tests.map(x=>x[0]+": "+t(x[1]||"not reported"));
 lines.push("WebGL2: "+t(webgl?"available":"unavailable"));
 lines.push(t('CPU threads reported: {count}',{count:navigator.hardwareConcurrency||t('unknown')}));
 const out=document.getElementById("mediaResult");out.textContent=lines.join("\n");out.style.display="block";
});
paint();

// Restore a closed session into the *current* Chrome Runtime window.
// the sessions restore API recreates an independent window, which is
// deliberately not what the Restore previous tabs button promises.
let restoringPrevious=false;
async function offerPreviousSession(){
 const panel=document.getElementById('previousSession');
 if(!chrome.sessions?.getRecentlyClosed||!chrome.tabs?.create)return;
 try{
  const entries=await chrome.sessions.getRecentlyClosed({maxResults:10});
  const previous=entries.find(entry=>entry.window?.tabs?.some(t=>/^https?:\/\//i.test(t.url||''))||/^https?:\/\//i.test(entry.tab?.url||''));
  if(!previous)return;
  const tabs=(previous.window?.tabs||[previous.tab]).filter(t=>/^https?:\/\//i.test(t?.url||'')).slice(0,100);
  if(!tabs.length)return;
  panel.hidden=false;
  const arabic=(navigator.language||'').toLowerCase().startsWith('ar');
  document.getElementById('sessionPrompt').textContent=arabic?'يمكنك استعادة التبويبات السابقة اختيارياً.':'You can restore your previous tabs if you wish.';
  document.getElementById('restorePrevious').textContent=arabic?'استعادة التبويبات السابقة':'Restore previous tabs';
  document.getElementById('restorePrevious').addEventListener('click',async()=>{
   if(restoringPrevious)return;
   restoringPrevious=true;
   const button=document.getElementById('restorePrevious');button.disabled=true;
   let created=0;
   try{
    const current=await chrome.windows.getCurrent();
    if(!Number.isInteger(current?.id)||current.type&&current.type!=='normal')throw new Error('No normal browser window');
    for(const tab of tabs){
     const target=created===0?tab.url:chrome.runtime.getURL('deferred.html')+'#'+encodeURIComponent(tab.url);
     await chrome.tabs.create({windowId:current.id,url:target,active:created===0,pinned:!!tab.pinned});
     created++;
    }
    panel.hidden=true;
   }catch{
    document.getElementById('sessionFeedback').textContent=arabic
      ?`تمت استعادة ${created} تبويبات؛ تعذر إكمال البقية.`
      :`Restored ${created} tabs; could not finish restoring the others.`;
   }finally{button.disabled=false;restoringPrevious=false;}
  });
 }catch{/* Sessions API may be unavailable in some CEF builds. */}
}
offerPreviousSession();
