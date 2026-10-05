#pragma once
#include <windows.h>
#include "include/cef_command_line.h"

namespace maenbrowser::resource {
struct ResourceProfile {
  bool lite = false;
  unsigned long long physical_mb = 0;
};
ResourceProfile DetectResourceProfile();
void ApplyResourcePolicy(CefRefPtr<CefCommandLine> command_line,
                         const ResourceProfile& profile);
}
