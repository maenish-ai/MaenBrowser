import {t,errorText} from './i18n.js';
export class NativeConnectionError extends Error {
  constructor(code){super(code);this.code=code;}
}
export async function nativeApi(op = 'get', extra = {}) {
  let response,data;
  try {
    response = await fetch('https://maen.browser/api', {
      method:'POST',headers:{'Content-Type':'application/json'},
      body:JSON.stringify({op,...extra}),cache:'no-store',signal:AbortSignal.timeout(8000)
    });
  }catch{throw new NativeConnectionError('PROTECTION_NETWORK');}
  if(!response.ok)throw new NativeConnectionError('PROTECTION_HTTP_'+response.status);
  try{data=await response.json();}catch{throw new NativeConnectionError('PROTECTION_RESPONSE');}
  if(!data||typeof data.ok!=='boolean')throw new NativeConnectionError('PROTECTION_RESPONSE');
  if(!data.ok)throw new Error(data.error||'Could not apply settings.');
  return data;
}
export async function api(op = 'get', extra = {}) {
  let result,timer;
  try{result=await Promise.race([
    chrome.runtime.sendMessage({op:'nativeApi',operation:op,extra}),
    new Promise((_,reject)=>{timer=setTimeout(()=>reject(new NativeConnectionError('PROTECTION_WORKER')),10000);})
  ]);}
  catch{result={ok:false,code:'PROTECTION_WORKER'};}
  finally{clearTimeout(timer);}
  if(!result)result={ok:false,code:'PROTECTION_WORKER'};
  let data;
  if(result.ok)data=result.data;
  else if(result.code){
    // Reading state is safe to repeat through the current extension frame.
    // Never replay a settings write: it might already have been applied.
    if(op!=='get')throw new NativeConnectionError(result.code);
    data=await nativeApi('get');
  }else throw new Error(result.error||'Could not connect to protection. Please retry or open Settings.');
  if(!data||data.ok!==true||typeof data.ads!=='boolean'||typeof data.family!=='boolean'||!Array.isArray(data.exceptions))
    throw new NativeConnectionError('PROTECTION_RESPONSE');
  // Language persistence is optional UI housekeeping, not a protection failure.
  if(op==='get'&&['en','ar'].includes(data.language)){
    // A stalled optional storage write must not stall the protection response.
    try{Promise.resolve(chrome.storage.local.set({language:data.language})).catch(error=>console.warn('Language preference could not be saved',error.name));}
    catch(error){console.warn('Language preference could not be saved',error.name);}
  }
  return data;
}
export function message(element, error) {
  element.textContent = error instanceof Error ? errorText(error) : t(String(error));
  element.classList.toggle('error', error instanceof Error);
}
export function host(url) { try { return new URL(url).hostname.toLowerCase().replace(/\.+$/, ''); } catch { return ''; } }
export function webUrl(url) { try { return ['https:', 'http:'].includes(new URL(url).protocol); } catch { return false; } }
export function matchesHost(value, domain) { return value === domain || value.endsWith('.' + domain); }
export const $ = id => document.getElementById(id);
