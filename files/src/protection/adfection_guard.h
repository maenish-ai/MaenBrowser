#pragma once
// Published, narrowly scoped AdFection indicators; independent of ad preferences.
// https://citizenlab.ca/research/how-discover-tech-turned-targeted-ads-into-a-pipeline-for-spyware/
// Citizen Lab (2026-10-15), section 2 and Appendix C.
// Never include entire ad exchanges, tracking networks, legitimate CDNs, Akedo,
// Volasim or MyIPInfo in this non-optional safety layer.
#include <array>
#include <string_view>
#include "src/protection/domain_rules.h"

namespace maenbrowser::protection {

inline bool KnownAdFectionHost(std::string_view host) {
  // Domains published as Candiru validator / command-and-control infrastructure.
  constexpr std::array<std::string_view, 6> indicators = {
      "keyadapter.com", "sharethere.net", "fearevolve.com",
      "insightenterprisells.com", "jobmarcher.com", "plasticbells.com"};
  for (const auto domain : indicators) {
    if (HostMatches(host, domain)) return true;
  }
  return false;
}

inline bool KnownAdFectionLoader(std::string_view host, std::string_view path) {
  // Exact documented code-delivery routes (NOT their entire ad network domains).
  struct Endpoint { std::string_view host; std::string_view path; };
  constexpr std::array<Endpoint, 8> endpoints = {{
      {"noas.mtrtb.com", "/getAd"},
      {"infinoas.mtrtb.com", "/getAd"},
      {"ppv2.mtrtb.com", "/getAd"},
      {"pqv2.mtrtb.com", "/getAd"},
      {"events.platc-tester.com", "/cplatform/getAd"},
      {"events.myweb3id.io", "/cplatform/getAd"},
      {"noas.powerium.net", "/getAd"},
      {"qa-noas.powerium.net", "/getAd"}
  }};
  for (const auto& endpoint : endpoints) {
    if (host == endpoint.host && path == endpoint.path) return true;
  }
  return false;
}

} // namespace maenbrowser::protection
