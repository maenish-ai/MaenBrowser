#include "src/media/windows_media_foundation.h"

#include <windows.h>
#include <mfapi.h>
#include <mferror.h>
#include <mfidl.h>
#include <wrl/client.h>

namespace maenbrowser::media {
namespace {
bool HasDecoder(const GUID& major_type, const GUID& subtype) {
  MFT_REGISTER_TYPE_INFO input{major_type, subtype};
  IMFActivate** activates = nullptr;
  UINT32 count = 0;
  const HRESULT hr = MFTEnumEx(MFT_CATEGORY_VIDEO_DECODER,
      MFT_ENUM_FLAG_SYNCMFT | MFT_ENUM_FLAG_ASYNCMFT | MFT_ENUM_FLAG_HARDWARE |
          MFT_ENUM_FLAG_LOCALMFT | MFT_ENUM_FLAG_SORTANDFILTER,
      &input, nullptr, &activates, &count);
  if (activates) {
    for (UINT32 i = 0; i < count; ++i) activates[i]->Release();
    CoTaskMemFree(activates);
  }
  return SUCCEEDED(hr) && count > 0;
}

bool HasAudioDecoder(const GUID& subtype) {
  MFT_REGISTER_TYPE_INFO input{MFMediaType_Audio, subtype};
  IMFActivate** activates = nullptr;
  UINT32 count = 0;
  const HRESULT hr = MFTEnumEx(MFT_CATEGORY_AUDIO_DECODER,
      MFT_ENUM_FLAG_SYNCMFT | MFT_ENUM_FLAG_ASYNCMFT | MFT_ENUM_FLAG_HARDWARE |
          MFT_ENUM_FLAG_LOCALMFT | MFT_ENUM_FLAG_SORTANDFILTER,
      &input, nullptr, &activates, &count);
  if (activates) {
    for (UINT32 i = 0; i < count; ++i) activates[i]->Release();
    CoTaskMemFree(activates);
  }
  return SUCCEEDED(hr) && count > 0;
}
}  // namespace

WindowsMediaCapabilities ProbeWindowsMediaFoundation() {
  WindowsMediaCapabilities out;
  const HRESULT hr = MFStartup(MF_VERSION, MFSTARTUP_LITE);
  if (FAILED(hr)) return out;
  out.media_foundation = true;
  out.h264_decoder = HasDecoder(MFMediaType_Video, MFVideoFormat_H264);
  out.aac_decoder = HasAudioDecoder(MFAudioFormat_AAC);
  MFShutdown();
  return out;
}
}  // namespace maenbrowser::media
