#include <windows.h>
#include <shellapi.h>
#include <wrl.h>
#include <wrl/event.h>
#include <string>
#include "WebView2.h"
#include "src/win/resource.h"

using Microsoft::WRL::Callback;
using Microsoft::WRL::ComPtr;

namespace {
ComPtr<ICoreWebView2Controller> g_controller;
ComPtr<ICoreWebView2> g_webview;
HWND g_hwnd = nullptr;

void ResizeWebView() {
  if (!g_controller || !g_hwnd) return;
  RECT bounds{};
  GetClientRect(g_hwnd, &bounds);
  g_controller->put_Bounds(bounds);
}

std::wstring InitialUrl() {
  int argc = 0;
  LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
  std::wstring url = L"https://web.whatsapp.com/";
  if (argv && argc > 1) {
    std::wstring candidate = argv[1];
    if (candidate.rfind(L"https://web.whatsapp.com", 0) == 0 ||
        candidate.rfind(L"https://whatsapp.com", 0) == 0) {
      url = candidate;
    }
  }
  if (argv) LocalFree(argv);
  return url;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
  switch (message) {
    case WM_SIZE:
      ResizeWebView();
      return 0;
    case WM_SETFOCUS:
      if (g_controller) g_controller->MoveFocus(COREWEBVIEW2_MOVE_FOCUS_REASON_PROGRAMMATIC);
      return 0;
    case WM_DESTROY:
      g_webview.Reset();
      if (g_controller) g_controller->Close();
      g_controller.Reset();
      PostQuitMessage(0);
      return 0;
  }
  return DefWindowProcW(hwnd, message, wparam, lparam);
}
}  // namespace

int APIENTRY wWinMain(HINSTANCE instance, HINSTANCE, LPWSTR, int show) {
  SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

  WNDCLASSEXW wc{sizeof(wc)};
  wc.lpfnWndProc = WindowProc;
  wc.hInstance = instance;
  wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
  wc.hIcon = LoadIcon(instance, MAKEINTRESOURCE(IDI_MAENBROWSER));
  wc.hIconSm = wc.hIcon;
  wc.lpszClassName = L"MaenBrowserMediaHost";
  RegisterClassExW(&wc);

  g_hwnd = CreateWindowExW(0, wc.lpszClassName, L"WhatsApp — MaenBrowser",
                           WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                           1280, 820, nullptr, nullptr, instance, nullptr);
  if (!g_hwnd) return 1;
  ShowWindow(g_hwnd, show == 0 ? SW_SHOWNORMAL : show);
  UpdateWindow(g_hwnd);

  const std::wstring url = InitialUrl();
  const std::wstring user_data = [] {
    wchar_t path[MAX_PATH]{};
    DWORD n = GetEnvironmentVariableW(L"LOCALAPPDATA", path, MAX_PATH);
    std::wstring base = (n > 0 && n < MAX_PATH) ? path : L".";
    return base + L"\\MaenBrowser\\WebView2 Media";
  }();

  HRESULT hr = CreateCoreWebView2EnvironmentWithOptions(
      nullptr, user_data.c_str(), nullptr,
      Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
          [url](HRESULT result, ICoreWebView2Environment* environment) -> HRESULT {
            if (FAILED(result) || !environment) {
              MessageBoxW(g_hwnd,
                          L"Microsoft Edge WebView2 Runtime is required for WhatsApp media. Re-run MaenBrowser Setup to install it.",
                          L"MaenBrowser Media", MB_OK | MB_ICONERROR);
              return result;
            }
            return environment->CreateCoreWebView2Controller(
                g_hwnd,
                Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                    [url](HRESULT controller_result, ICoreWebView2Controller* controller) -> HRESULT {
                      if (FAILED(controller_result) || !controller) return controller_result;
                      g_controller = controller;
                      g_controller->get_CoreWebView2(&g_webview);
                      ResizeWebView();
                      if (g_webview) {
                        ComPtr<ICoreWebView2Settings> settings;
                        if (SUCCEEDED(g_webview->get_Settings(&settings)) && settings) {
                          settings->put_IsScriptEnabled(TRUE);
                          settings->put_AreDefaultScriptDialogsEnabled(TRUE);
                          settings->put_IsWebMessageEnabled(TRUE);
                        }
                        g_webview->Navigate(url.c_str());
                      }
                      return S_OK;
                    }).Get());
          }).Get());

  if (FAILED(hr)) {
    MessageBoxW(g_hwnd,
                L"Could not start the Microsoft Edge WebView2 media engine.",
                L"MaenBrowser Media", MB_OK | MB_ICONERROR);
  }

  MSG msg{};
  while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
    TranslateMessage(&msg);
    DispatchMessageW(&msg);
  }
  return static_cast<int>(msg.wParam);
}
