#pragma once
#include <string_view>
#include "src/protection/identity.h"
namespace maenbrowser::protection {
// Inputs are CEF's request initiator and actual requesting frame URL, never a
// page-supplied header or the browser's top-level URL. Header-only authorization
// is retained solely for frameless requests with the exact forbidden Origin.
inline bool TrustedControlSource(std::string_view initiator,
                                std::string_view requesting_frame_url,
                                std::string_view origin_header) {
  if (!initiator.empty() && initiator != "null") return initiator == kExtensionOrigin;
  if (!requesting_frame_url.empty()) {
    constexpr std::string_view origin = kExtensionOrigin;
    return requesting_frame_url == origin ||
      (requesting_frame_url.starts_with(origin) &&
       requesting_frame_url.size() > origin.size() && requesting_frame_url[origin.size()] == '/');
  }
  return origin_header == kExtensionOrigin;
}
}
