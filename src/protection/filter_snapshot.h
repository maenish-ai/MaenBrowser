#pragma once
#include <sstream>
#include "src/protection/domain_rules.h"
namespace maenbrowser::protection {
inline bool ParseAdSnapshot(const std::string& text, DomainRules& rules) {
  if (text.empty() || text.size() > 4 * 1024 * 1024) return false;
  std::istringstream in(text); std::string line; std::vector<std::string> values;
  while (std::getline(in, line)) {
    if (!ValidDomain(line) || line.find('.') == std::string::npos ||
        (!values.empty() && values.back() >= line)) return false;
    values.push_back(line);
    if (values.size() > 250000) return false;
  }
  if (values.size() < 10000) return false;
  rules.Assign(std::move(values)); return true;
}
}
