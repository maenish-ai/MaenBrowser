#pragma once
#include <string_view>
#include "src/protection/adfection_guard.h"

namespace maenbrowser::protection {
// Hard-coded, reviewed indicators remain active if the installed text snapshot
// is absent or corrupted. The optional snapshot can add indicators at release.
inline bool SecurityThreatUrl(std::string_view host, std::string_view path,
                              const DomainRules& extra_threat_domains) {
  return KnownAdFectionHost(host) ||
         KnownAdFectionLoader(host, path) ||
         (!host.empty() && extra_threat_domains.Contains(host));
}
// CEF's OnBeforePopup returns true to CANCEL creation.
inline bool BlockUnsolicitedPopup(bool user_gesture) { return !user_gesture; }
} // namespace maenbrowser::protection
