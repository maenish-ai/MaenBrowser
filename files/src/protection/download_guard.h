#pragma once
// Lightweight local high-risk download classification. No background process or
// network lookups. This is a *confirmation* gate, not a malware scanner.
#include <array>
#include <cstddef>
#include <string_view>

namespace maenbrowser::protection {
inline bool RiskyDownloadName(std::wstring_view proposed_name) {
  const auto slash = proposed_name.find_last_of(L"/\\");
  if (slash != std::wstring_view::npos) proposed_name.remove_prefix(slash + 1);
  while (!proposed_name.empty() &&
         (proposed_name.back() == L'.' || proposed_name.back() == L' '))
    proposed_name.remove_suffix(1);
  const auto dot = proposed_name.rfind(L'.');
  if (dot == std::wstring_view::npos || dot == 0) return false;
  auto extension = proposed_name.substr(dot);
  constexpr std::array<std::wstring_view, 21> risky = {
      L".exe", L".com", L".msi", L".msp", L".msix", L".scr", L".bat",
      L".cmd", L".ps1", L".psm1", L".vbs", L".vbe", L".js", L".jse",
      L".wsf", L".wsh", L".hta", L".lnk", L".reg", L".jar", L".cpl"};
  for (const auto wanted : risky) {
    if (extension.size() != wanted.size()) continue;
    bool match = true;
    for (size_t i = 0; i < wanted.size(); ++i) {
      wchar_t c = extension[i];
      if (c >= L'A' && c <= L'Z') c += L'a' - L'A';
      if (c != wanted[i]) { match = false; break; }
    }
    if (match) return true;
  }
  return false;
}
} // namespace maenbrowser::protection
