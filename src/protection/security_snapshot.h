#pragma once
#include <charconv>
#include <cstdint>
#include <sstream>
#include "src/protection/domain_rules.h"
namespace maenbrowser::protection {
inline bool ParseSecuritySequence(const std::string& text, uint64_t& result) {
  if(text.empty() || text.size()>16 || text.front()=='0')return false;
  const auto [end,error]=std::from_chars(text.data(),text.data()+text.size(),result);
  return error==std::errc{} && end==text.data()+text.size() && result>0 && result<=9007199254740991ULL;
}
inline bool ParseSecurityDomains(const std::string& text, size_t minimum, DomainRules& rules) {
  if(text.empty() || text.size()>4*1024*1024 || text.back()!='\n')return false;
  std::istringstream input(text);std::string line;std::vector<std::string> domains;
  while(std::getline(input,line)) {
    if(!ValidDomain(line) || line.find('.')==std::string::npos || (!domains.empty() && domains.back()>=line))return false;
    domains.push_back(std::move(line));if(domains.size()>250000)return false;
  }
  if(domains.size()<minimum)return false;
  rules.Assign(std::move(domains));return true;
}
}
