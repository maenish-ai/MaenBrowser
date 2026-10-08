export async function api(op = 'get', extra = {}) {
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
export function message(element, error) {
  element.textContent = error instanceof Error ? error.message : String(error);
  element.classList.toggle('error', error instanceof Error);
}
export function host(url) { try { return new URL(url).hostname.toLowerCase().replace(/\.+$/, ''); } catch { return ''; } }
export function webUrl(url) { try { return ['https:', 'http:'].includes(new URL(url).protocol); } catch { return false; } }
export function matchesHost(value, domain) { return value === domain || value.endsWith('.' + domain); }
export const $ = id => document.getElementById(id);
