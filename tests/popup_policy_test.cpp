#include <cassert>
#include "src/app/popup_policy.h"
int main() {
  using namespace maenbrowser;
  assert(PopupHostMatches("www.foo.auth0.com", "auth0.com"));
  assert(!PopupHostMatches("evil-auth0.com", "auth0.com"));
  assert(ShouldPreservePopup("news.example.com", "accounts.google.com",
                             "https://news.example.com/", "https://accounts.google.com/"));
  assert(ShouldPreservePopup("news.example.com", "service.example.com",
                             "https://news.example.com/", "https://service.example.com/oauth/authorize"));
  assert(!ShouldPreservePopup("news.example.com", "example.net",
                              "https://news.example.com/", "https://example.net/article"));
}
