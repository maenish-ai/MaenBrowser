// Coalesce bursts of tab/download events while a control page is visible.
// This does not run in the browser background or touch web page contents.
export function scheduleRender(render,onError){
  let timer,dirty=false,running=false;
  const flush=async()=>{
    timer=undefined;
    if(document.hidden||running)return;
    dirty=false;running=true;
    try{await render();}catch(error){onError(error);}finally{running=false;if(dirty)queue();}
  };
  const queue=()=>{dirty=true;if(!timer&&!document.hidden&&!running)timer=setTimeout(flush,150);};
  document.addEventListener('visibilitychange',()=>{if(!document.hidden&&dirty)queue();});
  window.addEventListener('pagehide',()=>{clearTimeout(timer);});
  return queue;
}
