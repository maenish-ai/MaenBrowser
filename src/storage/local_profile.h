#pragma once

#include <string>

namespace maenbrowser::storage {
std::wstring GetAppDataRoot();
std::wstring GetProfileRoot();
std::wstring GetCachePath();
std::wstring GetExtensionsPath();
}  // namespace maenbrowser::storage
