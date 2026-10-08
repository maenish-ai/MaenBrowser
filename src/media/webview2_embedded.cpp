#include "src/media/webview2_embedded.h"

#include <windows.h>
#include <wrl.h>
#include <wrl/event.h>

#include <algorithm>
#include <cwctype>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "WebView2.h"

using Microsoft::WRL::Callback;
using Microsoft::WRL::ComPtr;

namespace maenbrowser::media {
namespace {

struct Surface {
  int browser_id = 0;
  HWND cef_window = nullptr;
  HWND content_host = nullptr;
  HWND child = nullptr;
  std::wstring pending_url;
  ComPtr<ICoreWebView2Controller> controller;
  ComPtr<ICoreWebView2> webview;
  EventRegistrationToken permission_token{};
  EventRegistrationToken new_window_token{};
};

ComPtr<ICoreWebView2Environment> g_environment;
bool g_environment_pending = false;
std::map<int, std::shared_ptr<Surface>> g_surfaces;
ATOM g_surface_class = 0;

bool StartsWithInsensitive(const std::wstring& value, const std::wstring& prefix) {
  if (value.size() < prefix.size()) return false;
  for (size_t i = 0; i < prefix.size(); ++i) {
    if (std::towlower(value[i]) != std::towlower(prefix[i])) return false;
  }
  return true;
}

bool IsTrustedMicrophoneOrigin(const std::wstring& uri) {
  return StartsWithInsensitive(uri, L"https://web.whatsapp.com/") ||
         StartsWithInsensitive(uri, L"https://web.whatsapp.com") ||
         StartsWithInsensitive(uri, L"https://whatsapp.com/") ||
         StartsWithInsensitive(uri, L"https://www.whatsapp.com/");
}

struct ContentCandidate {
  HWND hwnd = nullptr;
  LONG area = 0;
};

BOOL CALLBACK FindRenderHostProc(HWND hwnd, LPARAM lparam) {
  auto* best = reinterpret_cast<ContentCandidate*>(lparam);
  if (!best || !IsWindowVisible(hwnd)) return TRUE;
  wchar_t class_name[128]{};
  GetClassNameW(hwnd, class_name, 127);
  if (std::wstring(class_name).find(L"Chrome_RenderWidgetHostHWND") == std::wstring::npos)
    return TRUE;
  RECT r{};
  if (!GetClientRect(hwnd, &r)) return TRUE;
  LONG area = std::max<LONG>(0, r.right - r.left) * std::max<LONG>(0, r.bottom - r.top);
  if (area > best->area) {
    best->hwnd = hwnd;
    best->area = area;
  }
  return TRUE;
}

HWND FindContentHost(HWND cef_window) {
  if (!cef_window || !IsWindow(cef_window)) return nullptr;
  ContentCandidate best{};
  EnumChildWindows(cef_window, FindRenderHostProc, reinterpret_cast<LPARAM>(&best));
  return best.hwnd ? best.hwnd : cef_window;
}

void ResizeSurface(const std::shared_ptr<Surface>& surface) {
  if (!surface || !surface->child || !IsWindow(surface->child)) return;
  HWND parent = GetParent(surface->child);
  if (!parent || !IsWindow(parent)) return;
  RECT r{};
  GetClientRect(parent, &r);
  SetWindowPos(surface->child, HWND_TOP, 0, 0,
               std::max<LONG>(1, r.right - r.left),
               std::max<LONG>(1, r.bottom - r.top),
               SWP_NOACTIVATE | SWP_SHOWWINDOW);
  if (surface->controller) {
    RECT bounds{};
    GetClientRect(surface->child, &bounds);
    surface->controller->put_Bounds(bounds);
  }
}

LRESULT CALLBACK SurfaceProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
  const int browser_id = static_cast<int>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
  auto it = g_surfaces.find(browser_id);
  std::shared_ptr<Surface> surface = it == g_surfaces.end() ? nullptr : it->second;
  switch (msg) {
    case WM_TIMER:
      if (wp == 1 && surface) ResizeSurface(surface);
      return 0;
    case WM_SIZE:
      if (surface && surface->controller) {
        RECT bounds{};
        GetClientRect(hwnd, &bounds);
        surface->controller->put_Bounds(bounds);
      }
      return 0;
    case WM_SETFOCUS:
      if (surface && surface->controller)
        surface->controller->MoveFocus(COREWEBVIEW2_MOVE_FOCUS_REASON_PROGRAMMATIC);
      return 0;
    case WM_NCDESTROY:
      KillTimer(hwnd, 1);
      return DefWindowProcW(hwnd, msg, wp, lp);
  }
  return DefWindowProcW(hwnd, msg, wp, lp);
}

bool EnsureSurfaceClass() {
  if (g_surface_class) return true;
  WNDCLASSEXW wc{sizeof(wc)};
  wc.lpfnWndProc = SurfaceProc;
  wc.hInstance = GetModuleHandleW(nullptr);
  wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
  wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
  wc.lpszClassName = L"MaenBrowserEmbeddedWebView2";
  g_surface_class = RegisterClassExW(&wc);
  if (!g_surface_class && GetLastError() == ERROR_CLASS_ALREADY_EXISTS) g_surface_class = 1;
  return g_surface_class != 0;
}

std::wstring WebView2UserDataFolder() {
  wchar_t path[MAX_PATH]{};
  const DWORD n = GetEnvironmentVariableW(L"LOCALAPPDATA", path, MAX_PATH);
  std::wstring base = (n > 0 && n < MAX_PATH) ? path : L".";
  return base + L"\\MaenBrowser\\WebView2";
}

void ConfigureWebView(const std::shared_ptr<Surface>& surface) {
  if (!surface || !surface->webview) return;

  ComPtr<ICoreWebView2Settings> settings;
  if (SUCCEEDED(surface->webview->get_Settings(&settings)) && settings) {
    settings->put_IsScriptEnabled(TRUE);
    settings->put_AreDefaultScriptDialogsEnabled(TRUE);
    settings->put_IsWebMessageEnabled(TRUE);
    settings->put_AreDevToolsEnabled(FALSE);
  }

  surface->webview->add_PermissionRequested(
      Callback<ICoreWebView2PermissionRequestedEventHandler>(
          [](ICoreWebView2*, ICoreWebView2PermissionRequestedEventArgs* args) -> HRESULT {
            if (!args) return E_INVALIDARG;
            COREWEBVIEW2_PERMISSION_KIND kind{};
            if (FAILED(args->get_PermissionKind(&kind))) return S_OK;
            if (kind != COREWEBVIEW2_PERMISSION_KIND_MICROPHONE) return S_OK;

            LPWSTR raw_uri = nullptr;
            std::wstring uri;
            if (SUCCEEDED(args->get_Uri(&raw_uri)) && raw_uri) {
              uri = raw_uri;
              CoTaskMemFree(raw_uri);
            }
            // Let WhatsApp use microphones/headsets that Windows exposes to
            // WebView2. Windows privacy controls still remain authoritative.
            args->put_State(IsTrustedMicrophoneOrigin(uri)
                                ? COREWEBVIEW2_PERMISSION_STATE_ALLOW
                                : COREWEBVIEW2_PERMISSION_STATE_DENY);
            return S_OK;
          }).Get(),
      &surface->permission_token);

  surface->webview->add_NewWindowRequested(
      Callback<ICoreWebView2NewWindowRequestedEventHandler>(
          [weak = std::weak_ptr<Surface>(surface)](
              ICoreWebView2*, ICoreWebView2NewWindowRequestedEventArgs* args) -> HRESULT {
            auto current = weak.lock();
            if (!current || !current->webview || !args) return S_OK;
            LPWSTR raw_uri = nullptr;
            if (SUCCEEDED(args->get_Uri(&raw_uri)) && raw_uri) {
              current->webview->Navigate(raw_uri);
              CoTaskMemFree(raw_uri);
              args->put_Handled(TRUE);
            }
            return S_OK;
          }).Get(),
      &surface->new_window_token);
}

void CreateControllerForSurface(const std::shared_ptr<Surface>& surface) {
  if (!surface || !g_environment || surface->controller || !surface->child ||
      !IsWindow(surface->child))
    return;

  g_environment->CreateCoreWebView2Controller(
      surface->child,
      Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
          [weak = std::weak_ptr<Surface>(surface)](
              HRESULT result, ICoreWebView2Controller* controller) -> HRESULT {
            auto current = weak.lock();
            if (!current || FAILED(result) || !controller || !current->child ||
                !IsWindow(current->child))
              return result;

            current->controller = controller;
            current->controller->get_CoreWebView2(&current->webview);
            ResizeSurface(current);
            ConfigureWebView(current);
            if (current->webview && !current->pending_url.empty())
              current->webview->Navigate(current->pending_url.c_str());
            SetFocus(current->child);
            return S_OK;
          }).Get());
}

void StartEnvironmentIfNeeded() {
  if (g_environment || g_environment_pending) return;

  // WebView2 requires COM on the creating UI thread. CEF's Chrome Runtime owns
  // the message loop; initialize STA here only when the media engine is first
  // requested instead of paying the cost at browser startup.
  static bool com_checked = false;
  if (!com_checked) {
    const HRESULT com_hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(com_hr) && com_hr != RPC_E_CHANGED_MODE) {
      MessageBoxW(nullptr, L"Could not initialize the Windows COM apartment required by WebView2.",
                  L"MaenBrowser", MB_OK | MB_ICONERROR);
      return;
    }
    com_checked = true;
  }

  g_environment_pending = true;
  const std::wstring data_folder = WebView2UserDataFolder();
  const HRESULT hr = CreateCoreWebView2EnvironmentWithOptions(
      nullptr, data_folder.c_str(), nullptr,
      Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
          [](HRESULT result, ICoreWebView2Environment* environment) -> HRESULT {
            g_environment_pending = false;
            if (FAILED(result) || !environment) {
              MessageBoxW(nullptr,
                          L"Microsoft Edge WebView2 Runtime could not start. Re-run MaenBrowser Setup to repair the media engine.",
                          L"MaenBrowser", MB_OK | MB_ICONERROR);
              return result;
            }
            g_environment = environment;
            std::vector<std::shared_ptr<Surface>> waiting;
            waiting.reserve(g_surfaces.size());
            for (auto& [id, surface] : g_surfaces) {
              if (surface && !surface->controller) waiting.push_back(surface);
            }
            for (const auto& surface : waiting) CreateControllerForSurface(surface);
            return S_OK;
          }).Get());
  if (FAILED(hr)) {
    g_environment_pending = false;
    MessageBoxW(nullptr,
                L"Microsoft Edge WebView2 Runtime could not be initialized.",
                L"MaenBrowser", MB_OK | MB_ICONERROR);
  }
}

}  // namespace

bool OpenEmbeddedWebView2(int browser_id, HWND cef_window, const std::wstring& url) {
  if (browser_id <= 0 || !cef_window || !IsWindow(cef_window) || url.empty()) return false;
  if (!EnsureSurfaceClass()) return false;

  auto existing = g_surfaces.find(browser_id);
  if (existing != g_surfaces.end() && existing->second) {
    auto surface = existing->second;
    surface->pending_url = url;
    if (surface->webview) surface->webview->Navigate(url.c_str());
    if (surface->child) {
      ShowWindow(surface->child, SW_SHOW);
      ResizeSurface(surface);
      SetFocus(surface->child);
    }
    return true;
  }

  HWND content_host = FindContentHost(cef_window);
  if (!content_host || !IsWindow(content_host)) return false;

  auto surface = std::make_shared<Surface>();
  surface->browser_id = browser_id;
  surface->cef_window = cef_window;
  surface->content_host = content_host;
  surface->pending_url = url;
  surface->child = CreateWindowExW(
      0, L"MaenBrowserEmbeddedWebView2", L"", WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN,
      0, 0, 1, 1, content_host, nullptr, GetModuleHandleW(nullptr), nullptr);
  if (!surface->child) return false;

  SetWindowLongPtrW(surface->child, GWLP_USERDATA, static_cast<LONG_PTR>(browser_id));
  SetTimer(surface->child, 1, 500, nullptr);
  g_surfaces[browser_id] = surface;
  ResizeSurface(surface);

  if (g_environment) CreateControllerForSurface(surface);
  else StartEnvironmentIfNeeded();
  return true;
}

void CloseEmbeddedWebView2(int browser_id) {
  auto it = g_surfaces.find(browser_id);
  if (it == g_surfaces.end()) return;
  auto surface = it->second;
  g_surfaces.erase(it);
  if (!surface) return;
  if (surface->webview) {
    surface->webview->remove_PermissionRequested(surface->permission_token);
    surface->webview->remove_NewWindowRequested(surface->new_window_token);
  }
  surface->webview.Reset();
  if (surface->controller) surface->controller->Close();
  surface->controller.Reset();
  if (surface->child && IsWindow(surface->child)) DestroyWindow(surface->child);
  surface->child = nullptr;
}

}  // namespace maenbrowser::media
