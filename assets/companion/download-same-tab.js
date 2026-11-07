// Keep explicit same-origin HTML downloads in the existing tab. Do not replay
// a URL: the browser retains the original click, cookies and referrer. Forms,
// scripts, modified clicks, cross-origin links and named windows are untouched.
document.addEventListener('click', event => {
  if (!event.isTrusted || event.defaultPrevented || event.button !== 0 ||
      event.ctrlKey || event.metaKey || event.shiftKey || event.altKey) return;
  const anchor = event.target instanceof Element ? event.target.closest('a[download][target]') : null;
  if (!anchor || anchor.target.toLowerCase() !== '_blank' || !anchor.href) return;
  let url;
  try { url = new URL(anchor.href, document.baseURI); } catch { return; }
  if (url.origin !== location.origin || !['http:', 'https:', 'blob:'].includes(url.protocol)) return;
  const original = anchor.getAttribute('target');
  anchor.setAttribute('target', '_self');
  // Restore only if the site has not changed the target itself.
  setTimeout(() => {
    if (anchor.getAttribute('target') === '_self') anchor.setAttribute('target', original);
  }, 0);
}, false);
