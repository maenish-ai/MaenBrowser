// Do not detach sign-in/payment windows from their opener. OAuth relies on
// cookies, redirect state and sometimes window.opener/postMessage.
export const sensitivePopupHosts = [
  'accounts.google.com', 'login.microsoftonline.com', 'login.live.com',
  'appleid.apple.com', 'paypal.com', 'checkout.stripe.com',
  'auth0.com', 'okta.com', 'clerk.accounts.dev', 'cloudflareaccess.com',
  'claude.ai', 'anthropic.com',
];

function isAuthUrl(input) {
  try {
    const url = new URL(input);
    if (url.protocol !== 'https:' && url.protocol !== 'http:') return true;
    const host = url.hostname.toLowerCase();
    if (sensitivePopupHosts.some(s => host === s || host.endsWith('.' + s))) return true;
    const path = url.pathname.toLowerCase();
    if (/(?:^|\/)(?:auth|oauth2?|sso|oidc|login|log-in|signin|sign-in|authorize|callback|identity|consent|connect)(?:\/|$)/.test(path)) return true;
    const keys = ['redirect_uri', 'code_challenge', 'client_id', 'response_type', 'id_token_hint'];
    return keys.some(k => url.searchParams.has(k));
  } catch { return true; } // about:blank/invalid URLs can be OAuth pre-open windows.
}

export function shouldPreserveAuthPopup(destination, opener = '') {
  return isAuthUrl(destination) || (opener && isAuthUrl(opener));
}
