#pragma once
#include <string>
#include <string_view>
namespace maenbrowser {
// Match only explicit internal aliases, never a web URL containing these words.
inline bool IsAboutAlias(std::string_view input) {
  std::string url(input);
  for (auto& c : url) if (c >= 'A' && c <= 'Z') c += 'a' - 'A';
  url.erase(url.find_first_of("?#") == std::string::npos ? url.size() : url.find_first_of("?#"));
  if (!url.empty() && url.back() == '/') url.pop_back();
  return url == "chrome://settings/help" || url == "chrome://help" ||
         url == "chrome://about" || url == "chrome://version" ||
         url == "about:about" || url == "about:version" ||
         url == "maen://about" || url == "maen://technical-details";
}
}
