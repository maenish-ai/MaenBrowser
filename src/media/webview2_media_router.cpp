#include "src/media/webview2_media_router.h"
#include <cwctype>

namespace maenbrowser::media {
namespace {
std::wstring Lower(std::wstring value) {
  for (auto& ch : value) ch = static_cast<wchar_t>(std::towlower(ch));
  return value;
}
}

bool IsWhatsAppWebUrl(const std::wstring& url) {
  const auto lower = Lower(url);
  return lower.rfind(L"https://web.whatsapp.com", 0) == 0 ||
         lower.rfind(L"https://whatsapp.com", 0) == 0 ||
         lower.rfind(L"https://www.whatsapp.com", 0) == 0 ||
         lower.rfind(L"http://web.whatsapp.com", 0) == 0;
}
}  // namespace maenbrowser::media
