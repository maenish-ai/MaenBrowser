#include <windows.h>
#include <shellapi.h>
#include <wrl.h>
#include <wrl/event.h>
#include <WebView2.h>
#include <vector>
#include <memory>
#include <string>
#include <algorithm>
#include "../win/resource.h"
using Microsoft::WRL::ComPtr;
using Microsoft::WRL::Callback;
namespace {
constexpr int kBar=46, kTab=32;
constexpr int kAddress=100, kBack=101, kForward=102, kReload=103, kNew=104;
struct Tab { ComPtr<ICoreWebView2Controller> controller; ComPtr<ICoreWebView2> view; std::wstring title=L"New tab"; };
HWND windowHandle=nullptr, address=nullptr, tabsBar=nullptr;
std::vector<std::shared_ptr<Tab>> tabs;
int active=-1;
ComPtr<ICoreWebView2Environment> environment;
std::wstring userData;
void Layout();
void SwitchTo(int index) {
 if(index<0||index>=static_cast<int>(tabs.size()))return;
 active=index;
 for(int i=0;i<static_cast<int>(tabs.size());++i)if(tabs[i]->controller)tabs[i]->controller->put_IsVisible(i==active);
 LPWSTR uri=nullptr;
 if(tabs[active]->view&&SUCCEEDED(tabs[active]->view->get_Source(&uri))&&uri){SetWindowTextW(address,uri);CoTaskMemFree(uri);}
 InvalidateRect(tabsBar,nullptr,TRUE);Layout();
}
void NavigateText(){wchar_t buf[4096]{};GetWindowTextW(address,buf,4096);std::wstring u=buf;if(u.empty()||active<0)return;
 if(u.find(L"://")==std::wstring::npos){if(u.find(L'.')!=std::wstring::npos&&u.find(L' ')==std::wstring::npos)u=L"https://"+u;else {std::wstring q;for(wchar_t c:u){if(c==L' ')q+=L"%20";else q+=c;}u=L"https://www.google.com/search?q="+q;}}
 tabs[active]->view->Navigate(u.c_str());}
void AddTab(const std::wstring& initial=L"https://www.google.com/", ICoreWebView2NewWindowRequestedEventArgs* popup=nullptr) {
 if(!environment)return;
 auto tab=std::make_shared<Tab>();tabs.push_back(tab);const int index=static_cast<int>(tabs.size())-1;
 ComPtr<ICoreWebView2Deferral> deferral;
 if(popup)popup->GetDeferral(&deferral);
 ComPtr<ICoreWebView2NewWindowRequestedEventArgs> popupArgs=popup;
 environment->CreateCoreWebView2Controller(windowHandle,Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
 [tab,index,initial,popupArgs,deferral](HRESULT hr,ICoreWebView2Controller* controller)->HRESULT{
 if(FAILED(hr)||!controller){if(popupArgs)popupArgs->put_Handled(TRUE);if(deferral)deferral->Complete();return S_OK;}
 tab->controller=controller;controller->get_CoreWebView2(&tab->view);
 if(!tab->view){if(popupArgs)popupArgs->put_Handled(TRUE);if(deferral)deferral->Complete();return S_OK;}
 tab->view->add_NewWindowRequested(Callback<ICoreWebView2NewWindowRequestedEventHandler>(
 [](ICoreWebView2*,ICoreWebView2NewWindowRequestedEventArgs* args)->HRESULT{
   AddTab(L"",args);return S_OK;
 }).Get(),nullptr);
 tab->view->add_SourceChanged(Callback<ICoreWebView2SourceChangedEventHandler>(
 [tab](ICoreWebView2* sender,ICoreWebView2SourceChangedEventArgs*)->HRESULT{
 if(active>=0&&active<static_cast<int>(tabs.size())&&tabs[active]==tab){LPWSTR uri=nullptr;if(SUCCEEDED(sender->get_Source(&uri))&&uri){SetWindowTextW(address,uri);CoTaskMemFree(uri);}}return S_OK;
 }).Get(),nullptr);
 tab->view->add_DocumentTitleChanged(Callback<ICoreWebView2DocumentTitleChangedEventHandler>(
 [tab](ICoreWebView2* sender,IUnknown*)->HRESULT{LPWSTR t=nullptr;if(SUCCEEDED(sender->get_DocumentTitle(&t))&&t){tab->title=t;CoTaskMemFree(t);InvalidateRect(tabsBar,nullptr,TRUE);}return S_OK;
 }).Get(),nullptr);
 if(popupArgs){popupArgs->put_NewWindow(tab->view.Get());if(deferral)deferral->Complete();}
 else if(!initial.empty())tab->view->Navigate(initial.c_str());
 SwitchTo(index);return S_OK;
 }).Get());
}
void Layout(){if(!windowHandle)return;RECT r{};GetClientRect(windowHandle,&r);int w=r.right,h=r.bottom;
 if(tabsBar)MoveWindow(tabsBar,0,0,w,kTab,TRUE);
 const int top=kTab+5;
 HWND b=GetDlgItem(windowHandle,kBack);if(b)MoveWindow(b,5,top,34,32,TRUE);
 b=GetDlgItem(windowHandle,kForward);if(b)MoveWindow(b,43,top,34,32,TRUE);
 b=GetDlgItem(windowHandle,kReload);if(b)MoveWindow(b,81,top,38,32,TRUE);
 b=GetDlgItem(windowHandle,kNew);if(b)MoveWindow(b,w-42,top,36,32,TRUE);
 if(address)MoveWindow(address,124,top,std::max(80,w-174),32,TRUE);
 RECT area{0,kTab+kBar,w,h};for(auto& tab:tabs)if(tab->controller)tab->controller->put_Bounds(area);
}
LRESULT CALLBACK TabsProc(HWND h,UINT msg,WPARAM wp,LPARAM lp){
 if(msg==WM_LBUTTONDOWN){int x=GET_X_LPARAM(lp);int idx=x/170;if(idx>=0&&idx<static_cast<int>(tabs.size()))SwitchTo(idx);return 0;}
 if(msg==WM_PAINT){PAINTSTRUCT ps{};HDC dc=BeginPaint(h,&ps);RECT r{};GetClientRect(h,&r);FillRect(dc,&r,(HBRUSH)(COLOR_BTNFACE+1));SetBkMode(dc,TRANSPARENT);
 for(int i=0;i<static_cast<int>(tabs.size());++i){RECT tr{i*170+2,2,(i+1)*170-2,30};FillRect(dc,&tr,(HBRUSH)((i==active)?(COLOR_WINDOW+1):(COLOR_3DFACE+1)));std::wstring title=tabs[i]->title;DrawTextW(dc,title.c_str(),-1,&tr,DT_SINGLELINE|DT_VCENTER|DT_END_ELLIPSIS|DT_LEFT);}EndPaint(h,&ps);return 0;}
 return DefWindowProcW(h,msg,wp,lp);
}
LRESULT CALLBACK WindowProc(HWND h,UINT msg,WPARAM wp,LPARAM lp){switch(msg){
 case WM_SIZE:Layout();return 0;
 case WM_SETFOCUS:if(address)SetFocus(address);return 0;
 case WM_COMMAND:{int id=LOWORD(wp);if(id==kAddress&&HIWORD(wp)==EN_MAXTEXT)return 0;
 if(id==kNew){AddTab();return 0;}if(active<0||active>=static_cast<int>(tabs.size())||!tabs[active]->view)return 0;
 auto v=tabs[active]->view;if(id==kBack)v->GoBack();else if(id==kForward)v->GoForward();else if(id==kReload)v->Reload();return 0;}
 case WM_KEYDOWN:if(wp==VK_F5&&active>=0)tabs[active]->view->Reload();return 0;
 case WM_DESTROY:for(auto& t:tabs){t->view.Reset();if(t->controller)t->controller->Close();t->controller.Reset();}tabs.clear();environment.Reset();PostQuitMessage(0);return 0;
 }return DefWindowProcW(h,msg,wp,lp);}
LRESULT CALLBACK AddressProc(HWND h,UINT msg,WPARAM wp,LPARAM lp){if(msg==WM_KEYDOWN&&wp==VK_RETURN){NavigateText();return 0;}return CallWindowProcW((WNDPROC)GetPropW(h,L"MaenOldProc"),h,msg,wp,lp);}
}
int WINAPI wWinMain(HINSTANCE inst,HINSTANCE,LPWSTR,int show){HRESULT init=CoInitializeEx(nullptr,COINIT_APARTMENTTHREADED);if(FAILED(init))return 1;
 WNDCLASSW wc{};wc.hInstance=inst;wc.lpfnWndProc=WindowProc;wc.lpszClassName=L"MaenBrowserMain";wc.hCursor=LoadCursor(nullptr,IDC_ARROW);wc.hIcon=LoadIcon(inst,MAKEINTRESOURCE(IDI_MAENBROWSER));wc.hbrBackground=(HBRUSH)(COLOR_WINDOW+1);RegisterClassW(&wc);
 WNDCLASSW tc{};tc.hInstance=inst;tc.lpfnWndProc=TabsProc;tc.lpszClassName=L"MaenTabs";tc.hbrBackground=(HBRUSH)(COLOR_BTNFACE+1);RegisterClassW(&tc);
 windowHandle=CreateWindowW(wc.lpszClassName,L"MaenBrowser",WS_OVERLAPPEDWINDOW,CW_USEDEFAULT,CW_USEDEFAULT,1200,800,nullptr,nullptr,inst,nullptr);
 tabsBar=CreateWindowW(tc.lpszClassName,L"",WS_CHILD|WS_VISIBLE,0,0,100,kTab,windowHandle,nullptr,inst,nullptr);
 auto button=[&](const wchar_t* text,int id){CreateWindowW(L"BUTTON",text,WS_CHILD|WS_VISIBLE,0,0,40,30,windowHandle,(HMENU)(INT_PTR)id,inst,nullptr);};
 button(L"<",kBack);button(L">",kForward);button(L"R",kReload);button(L"+",kNew);
 address=CreateWindowExW(WS_EX_CLIENTEDGE,L"EDIT",L"",WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL,0,0,100,32,windowHandle,(HMENU)(INT_PTR)kAddress,inst,nullptr);
 SetPropW(address,L"MaenOldProc",(HANDLE)SetWindowLongPtrW(address,GWLP_WNDPROC,(LONG_PTR)AddressProc));
 ShowWindow(windowHandle,show);UpdateWindow(windowHandle);Layout();
 wchar_t local[MAX_PATH]{};GetEnvironmentVariableW(L"LOCALAPPDATA",local,MAX_PATH);userData=std::wstring(local)+L"\\MaenBrowser\\WebView2";
 HRESULT hr=CreateCoreWebView2EnvironmentWithOptions(nullptr,userData.c_str(),nullptr,Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
 [](HRESULT result,ICoreWebView2Environment* env)->HRESULT{if(FAILED(result)||!env){MessageBoxW(windowHandle,L"WebView2 Runtime is missing. Reinstall MaenBrowser.",L"MaenBrowser",MB_OK|MB_ICONERROR);return S_OK;}environment=env;AddTab();return S_OK;}).Get());
 if(FAILED(hr))MessageBoxW(windowHandle,L"Could not initialize WebView2.",L"MaenBrowser",MB_OK|MB_ICONERROR);
 MSG msg{};while(GetMessageW(&msg,nullptr,0,0)>0){TranslateMessage(&msg);DispatchMessageW(&msg);}CoUninitialize();return (int)msg.wParam;
}
