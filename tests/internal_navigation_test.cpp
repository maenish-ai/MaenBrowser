#include "src/app/internal_navigation.h"
#include <cassert>
int main() {
  assert(maenbrowser::IsTrustedInternalTab("chrome://history/"));
  assert(maenbrowser::IsTrustedInternalTab(std::string(maenbrowser::protection::kExtensionOrigin)+"/about.html"));
  for(const auto* url:{"chrome://extensions/","chrome://history.evil/","chrome://settings/","file:///tmp/a","javascript:alert(1)","https://evil.test/chrome://history/","chrome-extension://evil/about.html"})
    assert(!maenbrowser::IsTrustedInternalTab(url));
  assert(maenbrowser::IsSettingsAlias("chrome://settings/"));
  assert(maenbrowser::IsSettingsAlias("maen://settings"));
  assert(!maenbrowser::IsSettingsAlias("https://evil.test/chrome://settings"));
  assert(!maenbrowser::IsSettingsAlias("chrome://settings/content"));
  for (const auto* url : {"chrome://settings/help", "chrome://settings/help/", "CHROME://VERSION/", "maen://about", "maen://technical-details#details", "about:version", "chrome://about/?x=1"})
    assert(maenbrowser::IsAboutAlias(url));
  for (const auto* url : {"https://example.com/chrome://about", "https://maen.browser/about", "chrome://settings/helpful", "chrome://extensions", "chrome://settings/help/extra", "maen://about.evil", "about:blank", ""})
    assert(!maenbrowser::IsAboutAlias(url));
}
