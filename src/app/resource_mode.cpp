#include "src/app/resource_mode.h"

namespace maenbrowser::resource {
ResourceProfile DetectResourceProfile() {
  MEMORYSTATUSEX mem{};
  mem.dwLength = sizeof(mem);
  ResourceProfile p;
  if (GlobalMemoryStatusEx(&mem)) {
    p.physical_mb = mem.ullTotalPhys / (1024ull * 1024ull);
    // Automatic Lite Mode targets the machines MaenBrowser is designed to
    // rescue: 4 GB class PCs, while leaving modern PCs at Chromium defaults.
    p.lite = p.physical_mb > 0 && p.physical_mb <= 6144;
  }
  return p;
}

void ApplyResourcePolicy(CefRefPtr<CefCommandLine> cmd,
                         const ResourceProfile& p) {
  if (!cmd) return;
  cmd->AppendSwitch("disable-background-mode");
  cmd->AppendSwitch("disable-sync");
  if (!p.lite) return;

  // Lite Mode deliberately trades speculative speed for lower idle resource
  // use. It does NOT disable site isolation, the sandbox, Safe Browsing, TLS,
  // or other browser security boundaries.
  cmd->AppendSwitchWithValue("disable-features", "Prerender2,BackForwardCache");
  cmd->AppendSwitchWithValue("disk-cache-size", "134217728");
  cmd->AppendSwitchWithValue("media-cache-size", "67108864");
}
}
