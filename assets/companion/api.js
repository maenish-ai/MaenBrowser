import {t,errorText} from './i18n.js';
export async function nativeApi(op = 'get', extra = {}) {
  const response = await fetch('https://maen.browser/api', {
    method: 'POST', headers: {'Content-Type': 'application/json'},
    body: JSON.stringify({op, ...extra}), cache: 'no-store',
    signal: AbortSignal.timeout(15000)
  });
  if (!response.ok) throw new Error(`Native controls unavailable (${response.status}).`);
  const data = await response.json();
  if (!data.ok) throw new Error(data.error || 'Could not apply settings.');
  return data;
}
// Chrome action popups do not always have a CEF frame/request handler.
// The extension service worker owns the native connection for every UI surface.
export async function api(op = 'get', extra = {}) {
  const result=await chrome.runtime.sendMessage({op:'nativeApi',operation:op,extra});
  if(!result?.ok)throw new Error(result?.error||'Could not connect to protection. Please retry or open Settings.');
  if(op==='get'&&['en','ar'].includes(result.data.language))await chrome.storage.local.set({language:result.data.language});
  return result.data;
}
export function message(element, error) {
  element.textContent = error instanceof Error ? errorText(error) : t(String(error));
  element.classList.toggle('error', error instanceof Error);
}
export function host(url) { try { return new URL(url).hostname.toLowerCase().replace(/\.+$/, ''); } catch { return ''; } }
export function webUrl(url) { try { return ['https:', 'http:'].includes(new URL(url).protocol); } catch { return false; } }
export function matchesHost(value, domain) { return value === domain || value.endsWith('.' + domain); }
export const $ = id => document.getElementById(id);
