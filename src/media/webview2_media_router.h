#pragma once
#include <string>
namespace maenbrowser::media {
bool IsWhatsAppWebUrl(const std::wstring& url);
bool IsDirectMediaUrl(const std::wstring& url);
bool UsesEmbeddedMedia(const std::wstring& url);
}
