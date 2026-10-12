// Event-driven UI refresh: bounded frequency, no idle polling or overlap.
export function scheduleRender(render,onError,{minInterval=150}={}){
  const interval=Number.isFinite(minInterval)?Math.max(150,minInterval):150;
  let timer,dirty=false,running=false,suspended=false,lastStart=-Infinity;
  const visible=()=>!document.hidden&&!suspended;
  const flush=async()=>{
    timer=undefined;
    if(!visible()||running)return;
    dirty=false;running=true;lastStart=performance.now();
    try{await render();}catch(error){onError(error);}
    finally{running=false;if(dirty)queue();}
  };
  const queue=()=>{
    dirty=true;
    if(timer===undefined&&visible()&&!running){
      const delay=Math.max(150,interval-(performance.now()-lastStart));
      timer=setTimeout(flush,delay);
    }
  };
  const cancel=()=>{clearTimeout(timer);timer=undefined;};
  document.addEventListener('visibilitychange',()=>{
    if(!visible())cancel();else if(dirty)queue();
  });
  window.addEventListener('pagehide',()=>{suspended=true;cancel();dirty=true;});
  window.addEventListener('pageshow',()=>{suspended=false;if(dirty)queue();});
  return queue;
}
