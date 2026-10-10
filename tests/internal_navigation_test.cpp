#include "src/app/internal_navigation.h"
#include <cassert>
int main() {
  for (const auto* url : {"chrome://settings/help", "chrome://settings/help/", "CHROME://VERSION/", "maen://about", "maen://technical-details#details", "about:version", "chrome://about/?x=1"})
    assert(maenbrowser::IsAboutAlias(url));
  for (const auto* url : {"https://example.com/chrome://about", "https://maen.browser/about", "chrome://settings/helpful", "chrome://extensions", "chrome://settings/help/extra", "maen://about.evil", "about:blank", ""})
    assert(!maenbrowser::IsAboutAlias(url));
}
