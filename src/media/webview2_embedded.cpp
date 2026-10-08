#include "src/media/webview2_embedded.h"

#include <windows.h>
#include <commctrl.h>
#include <shobjidl.h>
#include <wrl.h>
#include <wrl/event.h>

#include <algorithm>
#include <cwctype>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "WebView2.h"
#include "include/cef_task.h"
#include "src/protection/protection.h"
#include "src/media/webview2_media_router.h"

using Microsoft::WRL::Callback;
using Microsoft::WRL::ComPtr;

namespace maenbrowser::media {
namespace {

class DownloadTask final : public CefTask {
 public:
  DownloadTask(std::function<void()> action, ComPtr<ICoreWebView2Deferral> deferral)
      : action_(std::move(action)), deferral_(std::move(deferral)) {}
  void Execute() override { action_(); deferral_->Complete(); }
 private:
  std::function<void()> action_;
  ComPtr<ICoreWebView2Deferral> deferral_;
  IMPLEMENT_REFCOUNTING(DownloadTask);
};

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
  EventRegistrationToken navigation_token{}, resource_token{}, download_token{};
  std::function<void(const std::wstring&)> navigate;
  bool parent_hook = false;
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
  return IsWhatsAppWebUrl(uri);
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

LRESULT CALLBACK ParentProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp,
                            UINT_PTR id, DWORD_PTR) {
  auto it = g_surfaces.find(static_cast<int>(id));
  if (it != g_surfaces.end() && (msg == WM_SIZE || msg == WM_WINDOWPOSCHANGED)) ResizeSurface(it->second);
  if (msg == WM_NCDESTROY) RemoveWindowSubclass(hwnd, ParentProc, id);
  return DefSubclassProc(hwnd, msg, wp, lp);
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
          [weak = std::weak_ptr<Surface>(surface)](ICoreWebView2*, ICoreWebView2PermissionRequestedEventArgs* args) -> HRESULT {
            if (!args) return E_INVALIDARG;
            COREWEBVIEW2_PERMISSION_KIND kind{};
            if (FAILED(args->get_PermissionKind(&kind))) return S_OK;
            if (kind != COREWEBVIEW2_PERMISSION_KIND_MICROPHONE && kind != COREWEBVIEW2_PERMISSION_KIND_CAMERA) return S_OK;

            LPWSTR raw_uri = nullptr;
            std::wstring uri;
            if (SUCCEEDED(args->get_Uri(&raw_uri)) && raw_uri) {
              uri = raw_uri;
              CoTaskMemFree(raw_uri);
            }
            auto current = weak.lock();
            bool allow = false;
            if (current && IsTrustedMicrophoneOrigin(uri) && !protection::FamilyEnabled()) {
              const wchar_t* question = kind == COREWEBVIEW2_PERMISSION_KIND_MICROPHONE
                  ? L"Allow WhatsApp Web to use your microphone for this request?"
                  : L"Allow WhatsApp Web to use your camera for this request?";
              allow = MessageBoxW(current->cef_window, question, L"MaenBrowser permission", MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) == IDYES;
            }
            args->put_State(allow ? COREWEBVIEW2_PERMISSION_STATE_ALLOW : COREWEBVIEW2_PERMISSION_STATE_DENY);
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
              if (!protection::BlockNavigation(CefString(raw_uri).ToString()) && current->navigate) current->navigate(raw_uri);
              CoTaskMemFree(raw_uri);
              args->put_Handled(TRUE);
            }
            return S_OK;
          }).Get(),
      &surface->new_window_token);

  surface->webview->add_NavigationStarting(
      Callback<ICoreWebView2NavigationStartingEventHandler>(
          [weak = std::weak_ptr<Surface>(surface)](ICoreWebView2*, ICoreWebView2NavigationStartingEventArgs* args) -> HRESULT {
            auto current = weak.lock(); if (!current || !args) return S_OK;
            LPWSTR raw = nullptr; args->get_Uri(&raw); if (!raw) return S_OK;
            std::wstring uri(raw); CoTaskMemFree(raw);
            if (protection::BlockNavigation(CefString(uri).ToString())) {
              args->put_Cancel(TRUE);
              MessageBoxW(current->cef_window, L"This address is blocked by Family Protection.", L"MaenBrowser", MB_OK | MB_ICONINFORMATION);
            } else if (!IsWhatsAppWebUrl(uri)) {
              args->put_Cancel(TRUE);
              if (current->navigate) current->navigate(uri);
            }
            return S_OK;
          }).Get(), &surface->navigation_token);

  surface->webview->AddWebResourceRequestedFilter(L"*", COREWEBVIEW2_WEB_RESOURCE_CONTEXT_ALL);
  surface->webview->add_WebResourceRequested(
      Callback<ICoreWebView2WebResourceRequestedEventHandler>(
          [weak = std::weak_ptr<Surface>(surface)](ICoreWebView2*, ICoreWebView2WebResourceRequestedEventArgs* args) -> HRESULT {
            auto current = weak.lock(); if (!current || !args || !g_environment) return S_OK;
            ComPtr<ICoreWebView2WebResourceRequest> request; args->get_Request(&request); if (!request) return S_OK;
            LPWSTR raw = nullptr; request->get_Uri(&raw); if (!raw) return S_OK;
            std::string url = CefString(raw).ToString(); CoTaskMemFree(raw);
            COREWEBVIEW2_WEB_RESOURCE_CONTEXT context{}; args->get_ResourceContext(&context);
            if (protection::Block(url, CefString(current->pending_url).ToString(), context == COREWEBVIEW2_WEB_RESOURCE_CONTEXT_DOCUMENT)) {
              ComPtr<ICoreWebView2WebResourceResponse> response;
              g_environment->CreateWebResourceResponse(nullptr, 403, L"Blocked by MaenBrowser", L"Cache-Control: no-store", &response);
              args->put_Response(response.Get());
            } else {
              auto rewritten = protection::Rewrite(url);
              if (rewritten != url) request->put_Uri(CefString(rewritten).ToWString().c_str());
            }
            return S_OK;
          }).Get(), &surface->resource_token);

  ComPtr<ICoreWebView2_4> downloads;
  if (SUCCEEDED(surface->webview.As(&downloads))) {
    downloads->add_DownloadStarting(
        Callback<ICoreWebView2DownloadStartingEventHandler>(
            [weak = std::weak_ptr<Surface>(surface)](ICoreWebView2*, ICoreWebView2DownloadStartingEventArgs* args) -> HRESULT {
              auto current = weak.lock();
              if (!args || !current) return S_OK;
              ComPtr<ICoreWebView2Deferral> deferral;
              if (FAILED(args->GetDeferral(&deferral))) { args->put_Cancel(TRUE); return S_OK; }
              ComPtr<ICoreWebView2DownloadStartingEventArgs> pending = args;
              // Return from the WebView2 event before displaying modal UI.
              const bool posted = CefPostTask(TID_UI, new DownloadTask([weak, pending]() {
                auto current = weak.lock();
                auto* args = pending.Get();
                if (!current || !IsWindow(current->cef_window)) { args->put_Cancel(TRUE); return; }
              LPWSTR raw = nullptr; args->get_ResultFilePath(&raw);
              std::wstring suggested = raw ? raw : L"download";
              CoTaskMemFree(raw);
              auto path = protection::DownloadPath(suggested);
              if (protection::Current()->ask_download || path.empty()) {
                ComPtr<IFileSaveDialog> dialog;
                HRESULT hr = CoCreateInstance(CLSID_FileSaveDialog, nullptr,
                    CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&dialog));
                if (SUCCEEDED(hr)) {
                  DWORD options = 0; dialog->GetOptions(&options);
                  dialog->SetOptions(options | FOS_FORCEFILESYSTEM | FOS_OVERWRITEPROMPT);
                  const std::filesystem::path initial(path.empty() ? suggested : path);
                  dialog->SetFileName(initial.filename().c_str());
                  ComPtr<IShellItem> folder;
                  if (!initial.parent_path().empty() && SUCCEEDED(SHCreateItemFromParsingName(
                      initial.parent_path().c_str(), nullptr, IID_PPV_ARGS(&folder))))
                    dialog->SetDefaultFolder(folder.Get());
                  hr = dialog->Show(current->cef_window);
                  ComPtr<IShellItem> result;
                  if (SUCCEEDED(hr)) hr = dialog->GetResult(&result);
                  LPWSTR selected = nullptr;
                  if (SUCCEEDED(hr)) hr = result->GetDisplayName(SIGDN_FILESYSPATH, &selected);
                  if (SUCCEEDED(hr) && selected) path = selected;
                  CoTaskMemFree(selected);
                }
                if (FAILED(hr)) { args->put_Cancel(TRUE); return; }
              }
              if (!path.empty()) args->put_ResultFilePath(path.c_str());

              }, deferral));
              if (!posted) { args->put_Cancel(TRUE); deferral->Complete(); }
              return S_OK;
            }).Get(), &surface->download_token);
  }
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
    if (FAILED(com_hr)) {
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
            if (g_surfaces.empty()) return S_OK;
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

bool OpenEmbeddedWebView2(int browser_id, HWND cef_window, const std::wstring& url,
                         std::function<void(const std::wstring&)> navigate) {
  if (browser_id <= 0 || !cef_window || !IsWindow(cef_window) || url.empty()) return false;
  if (!EnsureSurfaceClass()) return false;

  auto existing = g_surfaces.find(browser_id);
  if (existing != g_surfaces.end() && existing->second) {
    auto surface = existing->second;
    surface->pending_url = url;
    surface->navigate = std::move(navigate);
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
  surface->navigate = std::move(navigate);
  surface->child = CreateWindowExW(
      0, L"MaenBrowserEmbeddedWebView2", L"", WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN,
      0, 0, 1, 1, content_host, nullptr, GetModuleHandleW(nullptr), nullptr);
  if (!surface->child) return false;

  SetWindowLongPtrW(surface->child, GWLP_USERDATA, static_cast<LONG_PTR>(browser_id));
  g_surfaces[browser_id] = surface;
  surface->parent_hook = SetWindowSubclass(content_host, ParentProc, static_cast<UINT_PTR>(browser_id), 0) != FALSE;
  // Event-driven resize normally has no idle wakeups. Fall back only when the
  // native host cannot be subclassed on this CEF build.
  if (!surface->parent_hook) SetTimer(surface->child, 1, 2000, nullptr);
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
    surface->webview->remove_NavigationStarting(surface->navigation_token);
    surface->webview->remove_WebResourceRequested(surface->resource_token);
    ComPtr<ICoreWebView2_4> downloads;
    if (SUCCEEDED(surface->webview.As(&downloads))) downloads->remove_DownloadStarting(surface->download_token);
  }
  surface->webview.Reset();
  if (surface->controller) surface->controller->Close();
  surface->controller.Reset();
  if (surface->parent_hook && IsWindow(surface->content_host))
    RemoveWindowSubclass(surface->content_host, ParentProc, static_cast<UINT_PTR>(browser_id));
  if (surface->child && IsWindow(surface->child)) DestroyWindow(surface->child);
  surface->child = nullptr;
  if (g_surfaces.empty()) g_environment.Reset();
}

}  // namespace maenbrowser::media
