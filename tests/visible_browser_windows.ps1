param([Parameter(Mandatory=$true)][int]$BrowserProcessId)
Add-Type @'
using System;
using System.Runtime.InteropServices;
public static class MaenWindowCount {
  public delegate bool Callback(IntPtr window, IntPtr data);
  [DllImport("user32.dll")] public static extern bool EnumWindows(Callback callback, IntPtr data);
  [DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr window);
  [DllImport("user32.dll")] public static extern uint GetWindowThreadProcessId(IntPtr window, out uint processId);
  public static int Count(uint processId) {
    int count = 0;
    EnumWindows((window, data) => { uint id; GetWindowThreadProcessId(window, out id); if (id == processId && IsWindowVisible(window)) count++; return true; }, IntPtr.Zero);
    return count;
  }
}
'@
[MaenWindowCount]::Count([uint32]$BrowserProcessId)
