#pragma once
#include <string_view>
namespace maenbrowser::protection {
// Chromium net error codes: classify only, never retry or alter traffic.
inline std::string_view NetworkErrorCategory(int code) {
  if(code==0 || code==-3 || code==-20)return {}; // success, canceled, blocked by client
  if(code==-21)return "network_changed";
  if(code==-106)return "offline";
  if(code==-105 || code==-137 || (code<=-800 && code>=-899))return "dns";
  if(code==-130 || code==-111 || code==-131)return "proxy";
  if(code==-7 || code==-118)return "timeout";
  if(code<=-200 && code>=-299)return "certificate";
  return "other";
}
}
