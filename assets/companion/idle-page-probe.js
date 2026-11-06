// Serialized by chrome.scripting: keep this function self-contained. Fail closed
// on large/complex pages instead of allocating arrays for every form control.
// No polling, page hooks, network requests, or persistent content scripts.
export async function inspectIdlePage(){
  if(document.visibilityState==='visible'||document.hasFocus())return false;
  if(navigator.mediaSession?.playbackState==='playing')return false;
  if(document.querySelector('[contenteditable]:not([contenteditable="false"]),[role="textbox"],[role="log"],[role="chat"],iframe,frame,object,embed'))return false;
  // Cross-origin frames and shadow trees can hide drafts or live calls.
  // If privileged shadow inspection is unavailable, preserve the tab.
  const inspectShadow=globalThis.chrome?.dom?.openOrClosedShadowRoot;
  if(typeof inspectShadow!=='function')return false;
  try{
    const walker=document.createTreeWalker(document.documentElement,1);
    let node=document.documentElement,remaining=1024;
    while(node){
      if(--remaining<0||inspectShadow(node))return false;
      node=walker.nextNode();
    }
  }catch{return false;}
  let budget=256;
  const media=document.querySelectorAll('audio,video');
  if(media.length>budget)return false;
  for(const e of media){if(!e.paused||e.srcObject||e.seeking)return false;}
  const controls=document.querySelectorAll('input,textarea,select');
  if(controls.length>budget)return false;
  for(const e of controls){
    if(['hidden','button','submit','reset'].includes(e.type))continue;
    if(e.value||e.checked||(['INPUT','TEXTAREA'].includes(e.tagName)&&e.value!==e.defaultValue))return false;
  }
  // A saved camera/microphone grant protects a potential background call.
  try{for(const name of ['microphone','camera']){
    const p=await navigator.permissions.query({name});if(p.state==='granted')return false;
  }}catch{return false;}
  return true;
}
