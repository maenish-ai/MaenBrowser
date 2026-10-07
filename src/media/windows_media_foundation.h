#pragma once

namespace maenbrowser::media {
struct WindowsMediaCapabilities {
  bool media_foundation = false;
  bool h264_decoder = false;
  bool aac_decoder = false;
};

// Probes Windows' native Media Foundation decoders without installing codec
// packs or weakening Chromium security. This is diagnostics/capability plumbing;
// Chromium/CEF still owns HTML5/MSE/WebRTC playback.
WindowsMediaCapabilities ProbeWindowsMediaFoundation();
}  // namespace maenbrowser::media
