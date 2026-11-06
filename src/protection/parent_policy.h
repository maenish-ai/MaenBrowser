#pragma once
#include <string>
#include <vector>
namespace maenbrowser::protection {
inline bool RequiresParentPin(bool active, bool has_pin, const std::vector<std::string>& changed, bool new_pin) {
  if(active)return true;
  if(!has_pin)return false;
  if(new_pin)return true;
  for(const auto& key:changed)
    if(key=="family" || key=="allowOnly" || key=="adult" || key=="violence" || key=="safeSearch" || key=="allowed" || key=="blocked")return true;
  return false;
}
}
