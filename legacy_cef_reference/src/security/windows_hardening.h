#pragma once
namespace maenbrowser::security {
// Applies low-overhead Windows process mitigations before CEF initialization.
// Returns false only when a mitigation call itself fails; callers log but keep
// compatibility because Chromium/CEF may already enforce equivalent policies.
bool ApplyWindowsProcessHardening();
}