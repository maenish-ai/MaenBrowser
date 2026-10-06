#pragma once
#include <windows.h>
#include "include/cef_command_line.h"

namespace maenbrowser::resource {
enum class Mode { Lite, Balanced, Performance };
struct ResourceProfile {
  Mode mode = Mode::Balanced;
  bool lite = false; // compatibility with existing code/tests
  unsigned long long physical_mb = 0;
  unsigned int logical_processors = 1;
  bool constrained_hardware = false;
};
ResourceProfile DetectResourceProfile();
void ApplyResourcePolicy(CefRefPtr<CefCommandLine> command_line,
                         const ResourceProfile& profile);
}  // namespace maenbrowser::resource
