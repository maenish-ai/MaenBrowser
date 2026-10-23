#pragma once
#include <initializer_list>
#include <string>
#include <string_view>

namespace maenbrowser {
inline bool PopupHostMatches(std::string_view host, std::string_view suffix) {
  return host == suffix ||
      (host.size() > suffix.size() &&
       host.substr(host.size() - suffix.size()) == suffix &&
       host[host.size() - suffix.size() - 1] == '.');
}
inline bool AuthPopupHost(std::string_view host) {
  for (std::string_view item : {
      "accounts.google.com", "login.microsoftonline.com", "login.live.com",
      "appleid.apple.com", "paypal.com", "checkout.stripe.com",
      "auth0.com", "okta.com", "clerk.accounts.dev", "cloudflareaccess.com",
      "claude.ai", "anthropic.com"}) {
    if (PopupHostMatches(host, item)) return true;
  }
  return false;
}
inline bool AuthPopupUrl(std::string_view input) {
  std::string lower(input);
  for (auto& c : lower) if (c >= 'A' && c <= 'Z') c += 'a' - 'A';
  for (std::string_view marker : {
      "/oauth", "/auth/", "/authorize", "/login", "/log-in",
      "/signin", "/sign-in", "/sso", "/oidc", "/callback",
      "/consent", "/connect", "redirect_uri=", "client_id=",
      "code_challenge=", "response_type="}) {
    if (lower.find(marker) != std::string::npos) return true;
  }
  return false;
}
inline bool ShouldPreservePopup(std::string_view opener_host,
                               std::string_view target_host,
                               std::string_view opener_url,
                               std::string_view target_url) {
  return AuthPopupHost(opener_host) || AuthPopupHost(target_host) ||
         AuthPopupUrl(opener_url) || AuthPopupUrl(target_url);
}
} // namespace maenbrowser
