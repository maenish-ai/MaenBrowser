#include "src/app/resource_mode.h"

namespace maenbrowser::resource {
ResourceProfile DetectResourceProfile() {
  MEMORYSTATUSEX mem{};
  mem.dwLength = sizeof(mem);
  ResourceProfile p;
  if (GlobalMemoryStatusEx(&mem)) {
    p.physical_mb = mem.ullTotalPhys / (1024ull * 1024ull);
    if (p.physical_mb > 0 && p.physical_mb <= 6144) {
      p.mode = Mode::Lite;
      p.lite = true;
    } else if (p.physical_mb >= 16384) {
      p.mode = Mode::Performance;
    } else {
      p.mode = Mode::Balanced;
    }
  }
  return p;
}

void ApplyResourcePolicy(CefRefPtr<CefCommandLine> cmd,
                         const ResourceProfile& p) {
  if (!cmd) return;

  // Keep background work conservative on every machine. Never disable the
  // Chromium sandbox, site isolation, TLS validation or other core security.
  cmd->AppendSwitch("disable-background-mode");
  cmd->AppendSwitch("disable-sync");

  if (p.mode == Mode::Lite) {
    // Old/low-memory PCs: trade speculative navigation speed for lower RAM.
    cmd->AppendSwitchWithValue("disable-features", "Prerender2,BackForwardCache");
    cmd->AppendSwitchWithValue("disk-cache-size", "134217728");  // 128 MiB
    cmd->AppendSwitchWithValue("media-cache-size", "67108864");  // 64 MiB
  } else if (p.mode == Mode::Balanced) {
    // Mainstream machines retain BFCache but avoid prerender speculation.
    cmd->AppendSwitchWithValue("disable-features", "Prerender2");
    cmd->AppendSwitchWithValue("disk-cache-size", "268435456");  // 256 MiB
    cmd->AppendSwitchWithValue("media-cache-size", "134217728"); // 128 MiB
  }
  // Performance mode intentionally stays close to Chromium defaults.
}
}  // namespace maenbrowser::resource
