#include "src/app/resource_mode.h"
#include "src/storage/local_profile.h"
#include <filesystem>
#include <fstream>

namespace maenbrowser::resource {
ResourceProfile DetectResourceProfile() {
  MEMORYSTATUSEX mem{};
  mem.dwLength = sizeof(mem);
  SYSTEM_INFO sys{};
  GetNativeSystemInfo(&sys);

  ResourceProfile p;
  p.logical_processors = sys.dwNumberOfProcessors > 0 ? sys.dwNumberOfProcessors : 1;
  if (GlobalMemoryStatusEx(&mem)) {
    p.physical_mb = mem.ullTotalPhys / (1024ull * 1024ull);
  }

  // Treat low RAM or a very small CPU as constrained hardware.  The policy
  // deliberately does not guess GPU capability or force unsupported GPU
  // features; Chromium's own GPU blocklist/fallback logic remains authoritative.
  p.constrained_hardware =
      (p.physical_mb > 0 && p.physical_mb <= 8192) || p.logical_processors <= 4;

  std::string selected;
  std::ifstream mode_file(std::filesystem::path(storage::GetAppDataRoot()) / L"resource-mode.txt");
  char mode_hint[32]{}; mode_file.getline(mode_hint, sizeof(mode_hint));
  if (mode_file) selected = mode_hint;
  p.mode = ChooseMode(p.physical_mb, p.logical_processors, selected);
  p.lite = p.mode == Mode::UltraLite || p.mode == Mode::Lite;
  return p;
}

const char* ModeName(Mode mode) {
  switch (mode) {
    case Mode::UltraLite: return "UltraLite";
    case Mode::Lite: return "Lite";
    case Mode::Performance: return "Performance";
    default: return "Balanced";
  }
}

void ApplyResourcePolicy(CefRefPtr<CefCommandLine> cmd,
                         const ResourceProfile& p) {
  if (!cmd) return;

  // Low-overhead baseline.  Never trade sandboxing, site isolation or TLS
  // validation for memory.  Also do not disable GPU/WebGL/video decode: on old
  // PCs that often moves expensive work back to the CPU and makes media/3D worse.
  cmd->AppendSwitch("disable-background-mode");
  cmd->AppendSwitch("disable-sync");
  cmd->AppendSwitch("disable-default-apps");

  if (p.mode == Mode::Lite || p.mode == Mode::UltraLite) {
    cmd->AppendSwitchWithValue("disable-features",
                               "Prerender2,BackForwardCache,OptimizationHints,MediaRouter");

    // 4 GiB-class systems need tighter caches than 6 GiB systems. These are
    // cache ceilings, not hard limits on a page/game renderer.
    if (p.mode == Mode::UltraLite) {
      // Keep enough cache for normal browsing while avoiding large disk/cache
      // churn on 4 GiB-class machines.
      cmd->AppendSwitchWithValue("disk-cache-size", "25165824");  // 24 MiB
      cmd->AppendSwitchWithValue("media-cache-size", "12582912"); // 12 MiB
    } else {
      // Lite now covers 8 GiB-class PCs too. A single bounded cache profile
      // avoids extra cache growth without limiting JavaScript heap or renderers.
      cmd->AppendSwitchWithValue("disk-cache-size", "67108864");   // 64 MiB
      cmd->AppendSwitchWithValue("media-cache-size", "33554432"); // 32 MiB
    }
  } else if (p.mode == Mode::Balanced) {
    cmd->AppendSwitchWithValue("disable-features", "Prerender2");
    cmd->AppendSwitchWithValue("disk-cache-size", "268435456");  // 256 MiB
    cmd->AppendSwitchWithValue("media-cache-size", "134217728"); // 128 MiB
  }
  // Performance mode intentionally stays close to Chromium defaults so modern
  // hardware can use the browser engine's current GPU/media optimizations.
}
}  // namespace maenbrowser::resource
