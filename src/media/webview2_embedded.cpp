#include "src/media/webview2_embedded.h"
#include "src/app/ui_language.h"
#include "src/app/resource_mode.h"

#include <windows.h>
#include <commctrl.h>
#include <shobjidl.h>
#include <wrl.h>
#include <wrl/event.h>

#include <algorithm>
#include <cwctype>
#include <map>
#include <fstream>
#include <iterator>
#include <memory>
#include <string>
#include <vector>

#include "WebView2.h"
#include "WebView2EnvironmentOptions.h"
#include "src/protection/protection.h"
#include "src/media/webview2_media_router.h"

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
  EventRegistrationToken navigation_token{}, resource_token{}, download_token{}, completed_token{}, failed_token{};
  std::function<void(const std::wstring&)> navigate, address_changed;
  bool parent_hook = false, root_hook = false, refresh_queued = false;
  HWND root_window = nullptr;
  int visible = -1;
  RECT last_bounds{};
  bool bounds_known = false;
  bool starting = true, failed = false;
};

thread_local ComPtr<ICoreWebView2Environment> g_environment;
thread_local bool g_environment_pending = false;
thread_local unsigned long long g_environment_generation = 0;
thread_local std::map<int, std::shared_ptr<Surface>> g_surfaces;
thread_local ATOM g_surface_class = 0;

bool StartsWithInsensitive(const std::wstring& value, const std::wstring& prefix) {
  if (value.size() < prefix.size()) return false;
  for (size_t i = 0; i < prefix.size(); ++i) {
    if (std::towlower(value[i]) != std::towlower(prefix[i])) return false;
  }
  return true;
}

bool IsWebNavigation(const std::wstring& uri) {
  const auto parsed = protection::ParseUrl(CefString(uri).ToString());
  return parsed.valid && !parsed.host.empty() &&
      (parsed.scheme == "https" || parsed.scheme == "http");
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

constexpr UINT kRefreshSurface = WM_APP + 31;

void FailSurface(const std::shared_ptr<Surface>& surface) {
  if (!surface) return;
  surface->starting = false;
  surface->failed = true;
  KillTimer(surface->child, 2);
  if (surface->webview) surface->webview->Stop();
  if (surface->controller && SUCCEEDED(surface->controller->put_IsVisible(FALSE))) surface->visible = 0;
  InvalidateRect(surface->child, nullptr, TRUE);
}

void UpdateBounds(const std::shared_ptr<Surface>& surface) {
  if (!surface || !surface->controller) return;
  RECT bounds{};
  if (!GetClientRect(surface->child, &bounds)) return;
  if (!surface->bounds_known || !EqualRect(&bounds, &surface->last_bounds)) {
    if (SUCCEEDED(surface->controller->put_Bounds(bounds))) {
      surface->last_bounds = bounds;
      surface->bounds_known = true;
    }
  }
}

void ResizeSurface(const std::shared_ptr<Surface>& surface) {
  if (!surface || !surface->child || !IsWindow(surface->child)) return;
  HWND parent = GetParent(surface->child);
  if (!parent || !IsWindow(parent)) return;
  RECT r{}, child{};
  if (!GetClientRect(parent, &r) || !GetClientRect(surface->child, &child)) return;
  const LONG width = std::max<LONG>(1, r.right - r.left);
  const LONG height = std::max<LONG>(1, r.bottom - r.top);
  if (child.right != width || child.bottom != height)
    SetWindowPos(surface->child, nullptr, 0, 0, width, height,
                 SWP_NOACTIVATE | SWP_NOZORDER);
  UpdateBounds(surface);
  if (surface->controller) {
    const bool visible = !surface->failed && IsWindowVisible(parent) && !IsIconic(surface->root_window);
    if (surface->visible != static_cast<int>(visible) &&
        SUCCEEDED(surface->controller->put_IsVisible(visible ? TRUE : FALSE)))
      surface->visible = visible;
    // Hiding the controller lets WebView2 throttle rendering. Do not force
    // memory to disk or suspend WhatsApp messages, calls or active downloads.
  }
}

LRESULT CALLBACK ParentProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp,
                            UINT_PTR id, DWORD_PTR) {
  auto it = g_surfaces.find(static_cast<int>(id));
  if (it != g_surfaces.end() &&
      (msg == WM_SIZE || msg == WM_WINDOWPOSCHANGED || msg == WM_SHOWWINDOW)) {
    auto surface = it->second;
    // Defer until Windows has applied show/hide and coalesce resize bursts.
    if (!surface->refresh_queued && surface->child)
      surface->refresh_queued = PostMessageW(surface->child, kRefreshSurface, 0, 0) != FALSE;
  }
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
      if (wp == 2 && surface && surface->starting) FailSurface(surface);
      return 0;
    case WM_PAINT: {
      PAINTSTRUCT paint{};
      HDC dc = BeginPaint(hwnd, &paint);
      if (surface && (surface->starting || surface->failed)) {
        RECT rect{}; GetClientRect(hwnd, &rect); InflateRect(&rect, -24, -24);
        SetBkMode(dc, TRANSPARENT);
        const wchar_t* text = surface->failed
            ? ui::Text(L"MB-704: Media engine could not load the page. Check WebView2 Runtime, then use Retry. The original tab is unchanged.",
                       L"MB-704: تعذر تحميل الصفحة بمحرك الوسائط. تحقق من تثبيت WebView2 ثم اضغط إعادة المحاولة. التبويب الأصلي لم يتغير.")
            : ui::Text(L"Loading…", L"جارٍ التحميل…");
        DrawTextW(dc, text, -1, &rect, DT_CENTER | DT_WORDBREAK | DT_NOPREFIX);
      }
      EndPaint(hwnd, &paint); return 0;
    }
    case kRefreshSurface:
      if (surface) { surface->refresh_queued = false; ResizeSurface(surface); }
      return 0;
    case WM_SIZE:
      UpdateBounds(surface);
      return 0;
    case WM_SETFOCUS:
      if (surface && surface->controller)
        surface->controller->MoveFocus(COREWEBVIEW2_MOVE_FOCUS_REASON_PROGRAMMATIC);
      return 0;
    case WM_NCDESTROY:
      KillTimer(hwnd, 1);
      KillTimer(hwnd, 2);
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

  surface->webview->add_ProcessFailed(
      Callback<ICoreWebView2ProcessFailedEventHandler>(
          [weak = std::weak_ptr<Surface>(surface)](ICoreWebView2*, ICoreWebView2ProcessFailedEventArgs* args) -> HRESULT {
            COREWEBVIEW2_PROCESS_FAILED_KIND kind{};
            if (args && SUCCEEDED(args->get_ProcessFailedKind(&kind)) &&
                (kind == COREWEBVIEW2_PROCESS_FAILED_KIND_BROWSER_PROCESS_EXITED ||
                 kind == COREWEBVIEW2_PROCESS_FAILED_KIND_RENDER_PROCESS_EXITED))
              FailSurface(weak.lock());
            return S_OK;
          }).Get(), &surface->failed_token);

  ComPtr<ICoreWebView2Settings> settings;
  if (SUCCEEDED(surface->webview->get_Settings(&settings)) && settings) {
    settings->put_IsScriptEnabled(TRUE);
    settings->put_AreDefaultScriptDialogsEnabled(TRUE);
    settings->put_IsWebMessageEnabled(FALSE);
    settings->put_AreDevToolsEnabled(FALSE);
  }

  surface->webview->add_PermissionRequested(
      Callback<ICoreWebView2PermissionRequestedEventHandler>(
          [weak = std::weak_ptr<Surface>(surface)](ICoreWebView2*, ICoreWebView2PermissionRequestedEventArgs* args) -> HRESULT {
            if (!args) return E_INVALIDARG;
            // This compatibility window is for playback. Never inherit CEF
            // camera/microphone grants or prompt modally inside a COM callback.
            args->put_State(COREWEBVIEW2_PERMISSION_STATE_DENY);
            return S_OK;
          }).Get(),
      &surface->permission_token);

  surface->webview->add_NewWindowRequested(
      Callback<ICoreWebView2NewWindowRequestedEventHandler>(
          [weak = std::weak_ptr<Surface>(surface)](
              ICoreWebView2*, ICoreWebView2NewWindowRequestedEventArgs* args) -> HRESULT {
            auto current = weak.lock();
            if (!current || !current->webview || !args) return S_OK;
            args->put_Handled(TRUE);
            BOOL user_initiated = FALSE;
            args->get_IsUserInitiated(&user_initiated);
            if (!user_initiated) return S_OK;
            LPWSTR raw_uri = nullptr;
            if (SUCCEEDED(args->get_Uri(&raw_uri)) && raw_uri) {
              if (IsWebNavigation(raw_uri) && !protection::BlockNavigation(CefString(raw_uri).ToString()) && current->navigate) current->navigate(raw_uri);
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
            if (!IsWebNavigation(uri)) {
              args->put_Cancel(TRUE);
            } else if (protection::BlockNavigation(CefString(uri).ToString())) {
              args->put_Cancel(TRUE);
              // Do not enter a modal message loop from a WebView2 callback.
              SetWindowTextW(current->root_window, ui::Text(
                  L"MaenBrowser — address blocked by Family Protection",
                  L"MaenBrowser — العنوان محجوب بحماية الأسرة"));
            } else if (current->address_changed) {
              // Dedicated native media window: redirects remain in WebView2,
              // including their cookies/methods. Update its own address display.
              current->pending_url = uri;
              current->address_changed(uri);
            } else if (!(IsWhatsAppWebUrl(current->pending_url) && IsWhatsAppWebUrl(uri)) &&
                       uri != current->pending_url) {
              // All changed URLs (including redirects) go back through CEF so
              // the visible address, history and family checks stay truthful.
              args->put_Cancel(TRUE);
              if (current->navigate) current->navigate(uri);
            }
            return S_OK;
          }).Get(), &surface->navigation_token);

  surface->webview->add_NavigationCompleted(
      Callback<ICoreWebView2NavigationCompletedEventHandler>(
          [weak = std::weak_ptr<Surface>(surface)](ICoreWebView2*, ICoreWebView2NavigationCompletedEventArgs* args) -> HRESULT {
            auto current = weak.lock();
            if (!current || !args || current->failed) return S_OK;
            BOOL success = FALSE;
            args->get_IsSuccess(&success);
            if (!success) {
              // Do not ignore TLS/HTTP failures or leave an opaque empty child.
              FailSurface(current);
              return S_OK;
            }
            current->starting = false; current->failed = false;
            KillTimer(current->child, 2);
            ResizeSurface(current);
            if (!IsDirectMediaUrl(current->pending_url)) return S_OK;
            std::ifstream input(protection::InstallDirectory() / L"companion" / L"direct-player.js", std::ios::binary);
            if (!input) return S_OK;
            const std::string script((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
            std::wstring code = ui::Arabic() ? L"document.documentElement.lang='ar';" : L"document.documentElement.lang='en';";
            code += CefString(script).ToWString();
            current->webview->ExecuteScript(code.c_str(), nullptr);
            return S_OK;
          }).Get(), &surface->completed_token);

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
              if (args) args->put_Cancel(TRUE);
              // Downloading remains in the original CEF tab in this first
              // compatibility release; do not move WebView2 COM objects to CEF.
              if (auto current = weak.lock()) {
                if (IsWindow(current->root_window)) SetWindowTextW(current->root_window,
                    ui::Text(L"MaenBrowser — download from the original tab",
                             L"MaenBrowser — نزّل الملف من التبويب الأصلي"));
              }
              return S_OK;
            }).Get(), &surface->download_token);
  }
}

void CreateControllerForSurface(const std::shared_ptr<Surface>& surface) {
  if (!surface || surface->failed || !g_environment || surface->controller || !surface->child ||
      !IsWindow(surface->child))
    return;

  const HRESULT started = g_environment->CreateCoreWebView2Controller(
      surface->child,
      Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
          [weak = std::weak_ptr<Surface>(surface)](
              HRESULT result, ICoreWebView2Controller* controller) -> HRESULT {
            auto current = weak.lock();
            if (!current || current->failed) { if (controller) controller->Close(); return S_OK; }
            if (FAILED(result) || !controller || !current->child || !IsWindow(current->child)) {
              FailSurface(current);
              return S_OK;
            }

            current->controller = controller;
            if (FAILED(current->controller->get_CoreWebView2(&current->webview)) || !current->webview) {
              FailSurface(current); return S_OK;
            }
            ResizeSurface(current);
            ConfigureWebView(current);
            if (!current->pending_url.empty() && FAILED(current->webview->Navigate(current->pending_url.c_str())))
              FailSurface(current);
            if (IsWindowVisible(current->child)) SetFocus(current->child);
            return S_OK;
          }).Get());
  if (FAILED(started)) FailSurface(surface);
}

void StartEnvironmentIfNeeded() {
  if (g_environment || g_environment_pending) return;

  // The dedicated media window owns STA initialization and its message pump.
  // Never initialize WebView2 on an unknown CEF apartment.
  APTTYPE apartment{}; APTTYPEQUALIFIER qualifier{};
  if (FAILED(CoGetApartmentType(&apartment, &qualifier)) ||
      (apartment != APTTYPE_STA && apartment != APTTYPE_MAINSTA)) {
    for (const auto& [id, surface] : g_surfaces) FailSurface(surface);
    return;
  }
  const auto generation = ++g_environment_generation;
  g_environment_pending = true;
  const std::wstring data_folder = WebView2UserDataFolder();
  auto options = Microsoft::WRL::Make<CoreWebView2EnvironmentOptions>();
  options->put_Language(ui::Arabic()?L"ar":L"en-US");
  // The second engine previously ignored Lite mode entirely. Keep its optional
  // preloading/caches bounded too; retain GPU decoding and all security defaults.
  if (resource::DetectResourceProfile().lite)
    options->put_AdditionalBrowserArguments(
        L"--disable-features=Prerender2,BackForwardCache,OptimizationHints "
        L"--disk-cache-size=67108864 --media-cache-size=33554432");
  const HRESULT hr = CreateCoreWebView2EnvironmentWithOptions(
      nullptr, data_folder.c_str(), options.Get(),
      Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
          [generation](HRESULT result, ICoreWebView2Environment* environment) -> HRESULT {
            if (generation != g_environment_generation) return S_OK;
            g_environment_pending = false;
            if (FAILED(result) || !environment) {
              for (const auto& [id, surface] : g_surfaces) FailSurface(surface);
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
    for (const auto& [id, surface] : g_surfaces) FailSurface(surface);
  }
}

}  // namespace

bool OpenEmbeddedWebView2(int browser_id, HWND cef_window, const std::wstring& url,
                         std::function<void(const std::wstring&)> navigate,
                         std::function<void(const std::wstring&)> address_changed) {
  if (browser_id <= 0 || !cef_window || !IsWindow(cef_window) || url.empty()) return false;
  if (!EnsureSurfaceClass()) return false;

  auto existing = g_surfaces.find(browser_id);
  if (existing != g_surfaces.end() && existing->second) {
    auto surface = existing->second;
    surface->pending_url = url;
    surface->navigate = std::move(navigate);
  surface->address_changed = std::move(address_changed);
    if (surface->webview) surface->webview->Navigate(url.c_str());
    if (surface->child) {
      ShowWindow(surface->child, SW_SHOW);
      ResizeSurface(surface);
      if (IsWindowVisible(surface->child)) SetFocus(surface->child);
    }
    return true;
  }

  HWND content_host = FindContentHost(cef_window);
  if (!content_host || !IsWindow(content_host)) return false;

  auto surface = std::make_shared<Surface>();
  surface->browser_id = browser_id;
  surface->cef_window = cef_window;
  surface->content_host = content_host;
  surface->root_window = GetAncestor(cef_window, GA_ROOT);
  surface->pending_url = url;
  surface->navigate = std::move(navigate);
  surface->address_changed = std::move(address_changed);
  surface->child = CreateWindowExW(
      0, L"MaenBrowserEmbeddedWebView2", L"", WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN,
      0, 0, 1, 1, content_host, nullptr, GetModuleHandleW(nullptr), nullptr);
  if (!surface->child) return false;

  SetWindowLongPtrW(surface->child, GWLP_USERDATA, static_cast<LONG_PTR>(browser_id));
  g_surfaces[browser_id] = surface;
  // Startup watchdog only: no permanent polling after navigation completes.
  SetTimer(surface->child, 2, 45000, nullptr);
  surface->parent_hook = SetWindowSubclass(content_host, ParentProc, static_cast<UINT_PTR>(browser_id), 0) != FALSE;
  if (surface->root_window && surface->root_window != content_host)
    surface->root_hook = SetWindowSubclass(surface->root_window, ParentProc,
        static_cast<UINT_PTR>(browser_id), 0) != FALSE;
  // Event-driven resize normally has no idle wakeups. Fall back only when the
  // native host cannot be subclassed on this CEF build.
  if (!surface->parent_hook || (surface->root_window != content_host && !surface->root_hook)) SetTimer(surface->child, 1, 2000, nullptr);
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
  surface->failed = true;
  surface->starting = false;
  if (surface->webview) {
    surface->webview->remove_ProcessFailed(surface->failed_token);
    surface->webview->remove_PermissionRequested(surface->permission_token);
    surface->webview->remove_NewWindowRequested(surface->new_window_token);
    surface->webview->remove_NavigationStarting(surface->navigation_token);
    surface->webview->remove_NavigationCompleted(surface->completed_token);
    surface->webview->remove_WebResourceRequested(surface->resource_token);
    ComPtr<ICoreWebView2_4> downloads;
    if (SUCCEEDED(surface->webview.As(&downloads))) downloads->remove_DownloadStarting(surface->download_token);
  }
  surface->webview.Reset();
  if (surface->controller) surface->controller->Close();
  surface->controller.Reset();
  if (surface->parent_hook && IsWindow(surface->content_host))
    RemoveWindowSubclass(surface->content_host, ParentProc, static_cast<UINT_PTR>(browser_id));
  if (surface->root_hook && IsWindow(surface->root_window))
    RemoveWindowSubclass(surface->root_window, ParentProc, static_cast<UINT_PTR>(browser_id));
  if (surface->child && IsWindow(surface->child)) DestroyWindow(surface->child);
  surface->child = nullptr;
  if (g_surfaces.empty()) {
    g_environment.Reset();
    g_environment_pending = false;
    ++g_environment_generation; // Ignore a completion from an abandoned attempt.
  }
}

}  // namespace maenbrowser::media
