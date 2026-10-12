#pragma once
#include <string_view>
namespace maenbrowser::resource {
enum class Mode { UltraLite, Lite, Balanced, Performance };
inline Mode ChooseMode(unsigned long long memory_mb, unsigned processors, std::string_view preference) {
  if(preference=="ultra")return Mode::UltraLite;
  if(preference=="lite")return Mode::Lite;
  if(preference=="balanced")return Mode::Balanced;
  if(memory_mb>0&&memory_mb<=2304)return Mode::UltraLite;
  if((memory_mb>0&&memory_mb<=6144)||processors<=4)return Mode::Lite;
  if(memory_mb>=16384&&processors>=8)return Mode::Performance;
  return Mode::Balanced;
}
}
