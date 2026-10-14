#pragma once
#include <string_view>
namespace maenbrowser::resource {
enum class Mode { UltraLite, Lite, Balanced, Performance };
inline Mode ChooseMode(unsigned long long memory_mb, unsigned processors, std::string_view preference) {
  if(preference=="ultra")return Mode::UltraLite;
  if(preference=="lite")return Mode::Lite;
  if(preference=="balanced")return Mode::Balanced;
  // Auto mode is intentionally conservative on older PCs. 4 GiB-class
  // machines and 2-thread CPUs need the strictest policy; 8 GiB-class or
  // <=4-thread systems stay in Lite. Explicit user choices above still win.
  if((memory_mb>0&&memory_mb<=4096)||processors<=2)return Mode::UltraLite;
  if((memory_mb>0&&memory_mb<=8192)||processors<=4)return Mode::Lite;
  if(memory_mb>=16384&&processors>=8)return Mode::Performance;
  return Mode::Balanced;
}
}
