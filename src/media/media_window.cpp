#include "src/media/media_window.h"
#include "src/media/webview2_embedded.h"
#include "src/protection/protection.h"
#include "src/app/ui_language.h"
#include <windows.h>
#include <atomic>
#include <algorithm>
#include <memory>
#include <mutex>
#include <thread>
#include <utility>

namespace maenbrowser::media {
namespace {
constexpr UINT kNavigate = WM_APP + 70;
constexpr UINT kSource = WM_APP + 71;
constexpr int kSurface = 1;
struct Worker {
  std::thread thread;
  std::atomic<HWND> window{nullptr};
  std::atomic<bool> stop{false}, done{false}, closing{false};
  std::mutex mutex;
  std::wstring requested, source;
};
// Accessed only by the CEF UI thread; worker owns its shared lifetime.
std::shared_ptr<Worker> g_worker;
struct Window {
  std::shared_ptr<Worker> worker;
  HWND hwnd = nullptr, address = nullptr, content = nullptr, retry = nullptr;
  std::wstring url;
  bool closed = false;
};

void QueueUrl(const std::shared_ptr<Worker>& worker, const std::wstring& url) {
  std::lock_guard<std::mutex> lock(worker->mutex);
  worker->requested = url;
  if (const HWND hwnd = worker->window.load()) PostMessageW(hwnd, kNavigate, 0, 0);
}

void Navigate(Window* state) {
  if (!state || state->closed) return;
  // Retry is a fresh controller, never a failed controller reused silently.
  CloseEmbeddedWebView2(kSurface);
  SetWindowTextW(state->address, state->url.c_str());
  const auto parsed = protection::ParseUrl(CefString(state->url).ToString());
  if (!parsed.valid || parsed.host.empty() ||
      (parsed.scheme != "https" && parsed.scheme != "http") ||
      protection::BlockNavigation(CefString(state->url).ToString())) {
    SetWindowTextW(state->address, ui::Text(L"Address blocked by browser policy.", L"العنوان محجوب وفق إعدادات المتصفح."));
    return;
  }
  const auto worker = state->worker;
  if (!OpenEmbeddedWebView2(kSurface, state->content, state->url,
      [worker](const std::wstring& target) { QueueUrl(worker, target); },
      [worker](const std::wstring& target) {
        std::lock_guard<std::mutex> lock(worker->mutex);
        worker->source = target;
        if (HWND hwnd = worker->window.load()) PostMessageW(hwnd, kSource, 0, 0);
      })) {
    SetWindowTextW(state->address, ui::Text(L"MB-704: Media window could not start. Close and retry.", L"MB-704: تعذر بدء نافذة الوسائط. أغلقها وأعد المحاولة."));
  }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wp, LPARAM lp) {
  auto* state = reinterpret_cast<Window*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
  if (message == WM_NCCREATE) {
    state = static_cast<Window*>(reinterpret_cast<CREATESTRUCTW*>(lp)->lpCreateParams);
    state->hwnd = hwnd;
    SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(state));
  }
  if (!state) return DefWindowProcW(hwnd, message, wp, lp);
  switch (message) {
    case WM_CREATE:
      state->retry = CreateWindowW(L"BUTTON", ui::Text(L"Retry", L"إعادة المحاولة"),
          WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 4, 4, 120, 30, hwnd,
          reinterpret_cast<HMENU>(100), GetModuleHandleW(nullptr), nullptr);
      state->address = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", state->url.c_str(),
          WS_CHILD | WS_VISIBLE | ES_READONLY | ES_AUTOHSCROLL, 130, 4, 600, 30,
          hwnd, nullptr, GetModuleHandleW(nullptr), nullptr);
      state->content = CreateWindowW(L"STATIC", L"", WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN,
          0, 40, 100, 100, hwnd, nullptr, GetModuleHandleW(nullptr), nullptr);
      if (!state->retry || !state->address || !state->content) return -1;
      return 0;
    case WM_SIZE: {
      RECT rect{}; GetClientRect(hwnd, &rect);
      MoveWindow(state->address, 130, 4, (std::max)(1L, rect.right - 134), 30, TRUE);
      MoveWindow(state->content, 0, 40, rect.right, (std::max)(1L, rect.bottom - 40), TRUE);
      return 0;
    }
    case WM_COMMAND:
      if (LOWORD(wp) == 100) Navigate(state);
      return 0;
    case kSource: {
      std::wstring source;
      { std::lock_guard<std::mutex> lock(state->worker->mutex); source = state->worker->source; }
      if (!source.empty()) { state->url = source; SetWindowTextW(state->address, source.c_str()); }
      return 0;
    }
    case kNavigate: {
      std::wstring next;
      { std::lock_guard<std::mutex> lock(state->worker->mutex);
        next = std::exchange(state->worker->requested, L""); }
      if (!next.empty()) { state->url = std::move(next); Navigate(state); }
      return 0;
    }
    case WM_CLOSE:
      DestroyWindow(hwnd);
      return 0;
    case WM_DESTROY:
      state->closed = true;
      state->worker->closing.store(true);
      state->worker->window.store(nullptr);
      CloseEmbeddedWebView2(kSurface);
      PostQuitMessage(0);
      return 0;
  }
  return DefWindowProcW(hwnd, message, wp, lp);
}

void RunWindow(const std::shared_ptr<Worker>& worker) {
  const HRESULT com = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
  if (FAILED(com)) { worker->done.store(true); return; }
  {
    Window state; state.worker = worker;
    { std::lock_guard<std::mutex> lock(worker->mutex); state.url = worker->requested; }
    WNDCLASSW type{};
    type.lpfnWndProc = WindowProc;
    type.hInstance = GetModuleHandleW(nullptr);
    type.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    type.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    type.lpszClassName = L"MaenBrowserMediaWindow";
    if (RegisterClassW(&type) || GetLastError() == ERROR_CLASS_ALREADY_EXISTS) {
      HWND hwnd = CreateWindowW(type.lpszClassName,
          ui::Text(L"MaenBrowser — Media compatibility (separate session)", L"MaenBrowser — توافق الفيديو (جلسة مستقلة)"),
          WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, CW_USEDEFAULT, CW_USEDEFAULT, 1100, 760,
          nullptr, nullptr, type.hInstance, &state);
      if (hwnd) {
        worker->window.store(hwnd);
        if (worker->stop.load()) DestroyWindow(hwnd);
        else {
          ShowWindow(hwnd, SW_SHOW);
          PostMessageW(hwnd, kNavigate, 0, 0);
          MSG message{};
          while (GetMessageW(&message, nullptr, 0, 0) > 0) {
            TranslateMessage(&message); DispatchMessageW(&message);
          }
          if (IsWindow(hwnd)) DestroyWindow(hwnd);
        }
      }
    }
    // All COM references, pending callbacks and controller cleanup stay here.
    CloseEmbeddedWebView2(kSurface);
  }
  CoUninitialize();
  worker->done.store(true);
}
}  // namespace

bool OpenMediaWindow(const std::wstring& url) {
  const auto parsed = protection::ParseUrl(CefString(url).ToString());
  if (!parsed.valid || parsed.host.empty() ||
      (parsed.scheme != "https" && parsed.scheme != "http") ||
      protection::BlockNavigation(CefString(url).ToString())) return false;
  if (g_worker && !g_worker->done.load() && !g_worker->closing.load()) {
    QueueUrl(g_worker, url);
    if (HWND hwnd = g_worker->window.load()) { ShowWindow(hwnd, SW_RESTORE); SetForegroundWindow(hwnd); }
    return true;
  }
  if (g_worker && g_worker->thread.joinable()) g_worker->thread.join();
  g_worker = std::make_shared<Worker>();
  g_worker->requested = url;
  try { g_worker->thread = std::thread(RunWindow, g_worker); }
  catch (...) { g_worker.reset(); return false; }
  return true;
}

void ShutdownMediaWindow() {
  if (!g_worker) return;
  g_worker->stop.store(true);
  if (HWND hwnd = g_worker->window.load()) PostMessageW(hwnd, WM_CLOSE, 0, 0);
  if (g_worker->thread.joinable()) g_worker->thread.join();
  g_worker.reset();
}
}  // namespace maenbrowser::media
