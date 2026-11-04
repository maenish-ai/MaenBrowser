#pragma once

#include <string>

namespace maenbrowser::storage {
std::wstring GetAppDataRoot();
bool IsAgentMode();
std::wstring GetProfileRoot();
std::wstring GetCachePath();
std::wstring GetExtensionsPath();
}  // namespace maenbrowser::storage
