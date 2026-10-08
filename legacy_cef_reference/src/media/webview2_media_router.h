#pragma once
#include <string>
namespace maenbrowser::media {
bool IsWhatsAppWebUrl(const std::wstring& url);
bool OpenInMediaHost(const std::wstring& url);
}
