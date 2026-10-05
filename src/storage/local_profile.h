#pragma once

#include <string>

namespace maenbrowser::storage {

// Returns a per-user profile directory under %LOCALAPPDATA%.
// No MaenBrowser cloud account is required; browsing state remains local.
std::wstring GetProfileRoot();
std::wstring GetCachePath();
std::wstring GetExtensionsPath();

}  // namespace maenbrowser::storage
