#include "src/media/webview2_media_router.h"
#include <cwctype>
#include "src/protection/protection.h"

namespace maenbrowser::media {
namespace {
std::wstring Lower(std::wstring value) {
  for (auto& ch : value) ch = static_cast<wchar_t>(std::towlower(ch));
  return value;
}
}

bool IsWhatsAppWebUrl(const std::wstring& url) {
  const auto parsed = protection::ParseUrl(CefString(url).ToString());
  return parsed.scheme == "https" &&
         (parsed.host == "web.whatsapp.com" || parsed.host == "whatsapp.com" || parsed.host == "www.whatsapp.com");
}
}  // namespace maenbrowser::media
