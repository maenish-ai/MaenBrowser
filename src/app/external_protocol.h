#pragma once
#include <string_view>
#include <initializer_list>
namespace maenbrowser {
inline bool AllowedExternalUrl(std::string_view url) {
  if (url.size()>4096 || url.find_first_of("\"<>\\\r\n")!=std::string_view::npos) return false;
  for(unsigned char c:url)if(c<32||c==127)return false;
  for(std::string_view scheme:{"mailto:","tel:","zoommtg:","msteams:"})
    if(url.starts_with(scheme))return true;
  return false;
}
}
