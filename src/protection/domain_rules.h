#pragma once
#include <algorithm>
#include <cctype>
#include <string>
#include <string_view>
#include <vector>

namespace maenbrowser::protection {
inline bool ValidDomain(std::string_view s) {
  if (s.empty() || s.size() > 253 || s.front() == '.' || s.back() == '.') return false;
  size_t label = 0;
  for (unsigned char c : s) {
    if (c == '.') { if (!label || label > 63) return false; label = 0; }
    else { if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-')) return false; ++label; }
  }
  return label > 0 && label <= 63;
}
inline std::string CanonicalHost(std::string s) {
  std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
  while (!s.empty() && s.back() == '.') s.pop_back();
  return s;
}
inline bool HostMatches(std::string_view host, std::string_view domain) {
  return host == domain || (host.size() > domain.size() &&
      host[host.size() - domain.size() - 1] == '.' && host.ends_with(domain));
}
// Sorted contiguous strings avoid a hash-table node allocation per domain.
// Requests only visit suffixes of a canonical host; no per-request regex scan.
class DomainRules {
 public:
  void Assign(std::vector<std::string> values) {
    values.erase(std::remove_if(values.begin(), values.end(), [](const auto& s) { return !ValidDomain(s); }), values.end());
    // Bundled snapshots are already sorted; custom rules may not be.
    if (!std::is_sorted(values.begin(), values.end()))
      std::sort(values.begin(), values.end());
    values.erase(std::unique(values.begin(), values.end()), values.end());
    values_ = std::move(values);
  }
  bool Contains(std::string_view host) const {
    while (!host.empty()) {
      auto it = std::lower_bound(values_.begin(), values_.end(), host,
          [](const std::string& a, std::string_view b) { return a < b; });
      if (it != values_.end() && *it == host) return true;
      const auto dot = host.find('.');
      if (dot == std::string_view::npos) break;
      host.remove_prefix(dot + 1);
    }
    return false;
  }
  size_t Size() const { return values_.size(); }
  const std::vector<std::string>& Values() const { return values_; }
 private:
  std::vector<std::string> values_;
};
}
