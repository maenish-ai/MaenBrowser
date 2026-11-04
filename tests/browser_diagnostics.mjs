// Only endpoint metadata is retained. Never serialize request bodies, raw
// headers, query strings, cookies, arbitrary URLs or browser-console messages.
export function controlNetworkEvent(method, params, tracked){
  if(method==='Network.requestWillBeSent'){
    let url;try{url=new URL(params.request?.url);}catch{return null;}
    if(url.origin!=='https://maen.browser'||url.pathname!=='/api')return null;
    tracked.add(params.requestId);
    const headers=params.request.headers||{};
    const origin=Object.entries(headers).find(([key])=>key.toLowerCase()==='origin')?.[1];
    return {event:'request',method:params.request.method,origin:origin===undefined?'missing':origin==='null'?'null':String(origin).startsWith('chrome-extension://')?'extension':'other'};
  }
  if(!tracked.has(params.requestId))return null;
  if(method==='Network.responseReceived')return {event:'response',status:params.response.status};
  if(method==='Network.loadingFailed'){
    tracked.delete(params.requestId);
    return {event:'failure',error:params.errorText,blockedReason:params.blockedReason,corsError:params.corsErrorStatus?.corsError};
  }
  if(method==='Network.loadingFinished')tracked.delete(params.requestId);
  return null;
}
export async function waitForControlApi(read,{attempts=8,delay=()=>new Promise(resolve=>setTimeout(resolve,250))}={}){
  let error;
  for(let n=0;n<attempts;n++){
    let response;
    try{response=await read();}catch(e){error=e;if(n+1<attempts)await delay();continue;}
    if(response.status!==200||response.data?.ok!==true)
      throw Error('Internal API readiness rejected: HTTP '+response.status);
    return response;
  }
  throw error||Error('Internal API did not become ready');
}
