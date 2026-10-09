#pragma once
#include <algorithm>
#include <string>
#include <string_view>

namespace maenbrowser::media {
// Input is a parsed URL path, never the query or fragment.
inline bool IsDirectMediaPath(std::string_view scheme, std::string path) {
  if (scheme != "https" || path.empty() || path.front() != '/' ||
      path.find_first_of("?#") != std::string::npos) return false;
  std::transform(path.begin(), path.end(), path.begin(), [](unsigned char c) {
    return c >= 'A' && c <= 'Z' ? static_cast<char>(c + ('a' - 'A')) : static_cast<char>(c);
  });
  for (std::string_view extension : {".mp4", ".m4v", ".m4a", ".aac"})
    if (path.size() > extension.size() && path.ends_with(extension)) return true;
  return false;
}
}
