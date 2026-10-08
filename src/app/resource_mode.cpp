#include "src/app/resource_mode.h"

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
      (p.physical_mb > 0 && p.physical_mb <= 6144) || p.logical_processors <= 4;

  if (p.physical_mb > 0 && p.physical_mb <= 6144) {
    p.mode = Mode::Lite;
    p.lite = true;
  } else if (p.physical_mb >= 16384 && p.logical_processors >= 8) {
    p.mode = Mode::Performance;
  } else {
    p.mode = Mode::Balanced;
  }
  return p;
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

  if (p.mode == Mode::Lite) {
    cmd->AppendSwitchWithValue("disable-features",
                               "Prerender2,BackForwardCache,OptimizationHints,MediaRouter");

    // 4 GiB-class systems need tighter caches than 6 GiB systems. These are
    // cache ceilings, not hard limits on a page/game renderer.
    if (p.physical_mb > 0 && p.physical_mb <= 4096) {
      cmd->AppendSwitchWithValue("disk-cache-size", "67108864");   // 64 MiB
      cmd->AppendSwitchWithValue("media-cache-size", "33554432"); // 32 MiB
    } else {
      cmd->AppendSwitchWithValue("disk-cache-size", "100663296");  // 96 MiB
      cmd->AppendSwitchWithValue("media-cache-size", "50331648");  // 48 MiB
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
