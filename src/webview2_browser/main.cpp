#include <windows.h>
#include <shellapi.h>
#include <windowsx.h>
#include <cwctype>
#include <filesystem>
#include <wrl.h>
#include <wrl/event.h>
#include <WebView2.h>
#include <vector>
#include <memory>
#include <string>
#include <algorithm>
#include <commctrl.h>
#include "../win/resource.h"
using Microsoft::WRL::ComPtr;
using Microsoft::WRL::Callback;
namespace {
constexpr int kBar=54, kTab=42;
constexpr int kAddress=100, kBack=101, kForward=102, kReload=103, kNew=104, kHome=105;
constexpr COLORREF kBackground=RGB(245,247,251),kInk=RGB(34,43,60),kAccent=RGB(229,237,255);
HBRUSH uiBrush=nullptr; HFONT uiFont=nullptr; WNDPROC oldAddressProc=nullptr;
void CloseTab(int index);
struct Tab { ComPtr<ICoreWebView2Controller> controller; ComPtr<ICoreWebView2> view; std::wstring title=L"New tab"; };
HWND windowHandle=nullptr, address=nullptr, tabsBar=nullptr;
std::vector<std::shared_ptr<Tab>> tabs;
int active=-1;
ComPtr<ICoreWebView2Environment> environment;
std::wstring userData;
std::wstring HomeUrl(){
 wchar_t path[MAX_PATH]{};
 if(!GetModuleFileNameW(nullptr,path,MAX_PATH))return L"https://www.google.com/";
 std::filesystem::path home=std::filesystem::path(path).parent_path()/L"assets"/L"maen_start.html";
 if(!std::filesystem::exists(home))return L"https://www.google.com/";
 std::wstring uri=L"file:///"+home.wstring();
 std::replace(uri.begin(),uri.end(),L'\\',L'/');
 return uri;
}

void Layout();
void SwitchTo(int index) {
 if(index<0||index>=static_cast<int>(tabs.size()))return;
 active=index;
 for(int i=0;i<static_cast<int>(tabs.size());++i)if(tabs[i]->controller)tabs[i]->controller->put_IsVisible(i==active);
 LPWSTR uri=nullptr;
 if(tabs[active]->view&&SUCCEEDED(tabs[active]->view->get_Source(&uri))&&uri){SetWindowTextW(address,uri);CoTaskMemFree(uri);}
 InvalidateRect(tabsBar,nullptr,TRUE);Layout();
}
void NavigateText(){wchar_t buf[4096]{};GetWindowTextW(address,buf,4096);std::wstring u=buf;if(u.empty()||active<0||active>=static_cast<int>(tabs.size())||!tabs[active]->view)return;
 if(u.find(L"://")==std::wstring::npos){if(u.find(L'.')!=std::wstring::npos&&u.find(L' ')==std::wstring::npos)u=L"https://"+u;else {std::wstring q;for(wchar_t c:u){if(c==L' ')q+=L"%20";else q+=c;}u=L"https://www.google.com/search?q="+q;}}
 tabs[active]->view->Navigate(u.c_str());}
void AddTab(const std::wstring& initial=HomeUrl(), ICoreWebView2NewWindowRequestedEventArgs* popup=nullptr) {
 if(!environment)return;
 auto tab=std::make_shared<Tab>();tabs.push_back(tab);const int index=static_cast<int>(tabs.size())-1;
 ComPtr<ICoreWebView2Deferral> deferral;
 if(popup && FAILED(popup->GetDeferral(&deferral))) { popup->put_Handled(TRUE); tabs.pop_back(); return; }
 ComPtr<ICoreWebView2NewWindowRequestedEventArgs> popupArgs=popup;
 environment->CreateCoreWebView2Controller(windowHandle,Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
 [tab,index,initial,popupArgs,deferral](HRESULT hr,ICoreWebView2Controller* controller)->HRESULT{
 if(FAILED(hr)||!controller){if(popupArgs)popupArgs->put_Handled(TRUE);if(deferral)deferral->Complete();tabs.erase(std::remove(tabs.begin(),tabs.end(),tab),tabs.end());return S_OK;}
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
 if(popupArgs){HRESULT popupResult=popupArgs->put_NewWindow(tab->view.Get());if(FAILED(popupResult))popupArgs->put_Handled(TRUE);if(deferral)deferral->Complete();}
 else if(!initial.empty())tab->view->Navigate(initial.c_str());
 auto it=std::find(tabs.begin(),tabs.end(),tab);
 if(it!=tabs.end())SwitchTo(static_cast<int>(it-tabs.begin()));return S_OK;
 }).Get());
}
void CloseTab(int index){
 if(index<0||index>=static_cast<int>(tabs.size()))return;
 auto tab=tabs[index];
 if(tab->controller)tab->controller->Close();
 tab->view.Reset();tab->controller.Reset();
 tabs.erase(tabs.begin()+index);
 if(tabs.empty()){active=-1;AddTab();return;}
 SwitchTo(std::min(index,static_cast<int>(tabs.size())-1));
}
void Layout(){if(!windowHandle)return;RECT r{};GetClientRect(windowHandle,&r);int w=r.right,h=r.bottom;
 if(tabsBar)MoveWindow(tabsBar,0,0,w,kTab,TRUE);
 const int top=kTab+10;
 HWND b=GetDlgItem(windowHandle,kBack);if(b)MoveWindow(b,12,top,38,34,TRUE);
 b=GetDlgItem(windowHandle,kForward);if(b)MoveWindow(b,54,top,38,34,TRUE);
 b=GetDlgItem(windowHandle,kReload);if(b)MoveWindow(b,96,top,38,34,TRUE);
 b=GetDlgItem(windowHandle,kNew);if(b)MoveWindow(b,w-52,top,40,34,TRUE);
 b=GetDlgItem(windowHandle,kHome);if(b)MoveWindow(b,138,top,65,34,TRUE);
 if(address)MoveWindow(address,210,top,std::max(80,w-274),34,TRUE);
 RECT area{0,kTab+kBar,w,h};for(auto& tab:tabs)if(tab->controller)tab->controller->put_Bounds(area);
}
LRESULT CALLBACK TabsProc(HWND h,UINT msg,WPARAM wp,LPARAM lp){
 if(msg==WM_LBUTTONDOWN){int x=GET_X_LPARAM(lp);int idx=x/190;if(idx>=0&&idx<static_cast<int>(tabs.size())){if(x%190>=166)CloseTab(idx);else SwitchTo(idx);}return 0;}
 if(msg==WM_PAINT){PAINTSTRUCT ps{};HDC dc=BeginPaint(h,&ps);RECT r{};GetClientRect(h,&r);FillRect(dc,&r,uiBrush);SetBkMode(dc,TRANSPARENT);SetTextColor(dc,kInk);
 if(uiFont)SelectObject(dc,uiFont);
 for(int i=0;i<static_cast<int>(tabs.size());++i){RECT tr{i*190+5,5,(i+1)*190-5,39};HBRUSH tabBrush=CreateSolidBrush(i==active?RGB(255,255,255):kAccent);
 FillRect(dc,&tr,tabBrush);DeleteObject(tabBrush);
 RECT label{tr.left+12,tr.top,tr.right-28,tr.bottom};std::wstring title=tabs[i]->title;DrawTextW(dc,title.c_str(),-1,&label,DT_SINGLELINE|DT_VCENTER|DT_END_ELLIPSIS|DT_LEFT);
 RECT close{tr.right-24,tr.top,tr.right-4,tr.bottom};DrawTextW(dc,L"×",-1,&close,DT_SINGLELINE|DT_CENTER|DT_VCENTER);}EndPaint(h,&ps);return 0;}
 return DefWindowProcW(h,msg,wp,lp);
}
LRESULT CALLBACK WindowProc(HWND h,UINT msg,WPARAM wp,LPARAM lp){switch(msg){
 case WM_SIZE:Layout();return 0;
 case WM_CTLCOLORSTATIC:case WM_CTLCOLOREDIT:{HDC dc=(HDC)wp;SetBkColor(dc,RGB(255,255,255));SetTextColor(dc,kInk);return (LRESULT)GetStockObject(WHITE_BRUSH);}
 case WM_CTLCOLORBTN:{HDC dc=(HDC)wp;SetBkColor(dc,kBackground);SetTextColor(dc,kInk);return (LRESULT)uiBrush;}
 case WM_ERASEBKGND:{RECT r{};GetClientRect(h,&r);FillRect((HDC)wp,&r,uiBrush);return 1;}
 case WM_SETFOCUS:if(address)SetFocus(address);return 0;
 case WM_COMMAND:{int id=LOWORD(wp);if(id==kAddress&&HIWORD(wp)==EN_MAXTEXT)return 0;
 if(id==kNew){AddTab();return 0;}if(active<0||active>=static_cast<int>(tabs.size())||!tabs[active]->view)return 0;
 auto v=tabs[active]->view;if(id==kBack){BOOL can=FALSE;if(SUCCEEDED(v->get_CanGoBack(&can))&&can)v->GoBack();}else if(id==kForward){BOOL can=FALSE;if(SUCCEEDED(v->get_CanGoForward(&can))&&can)v->GoForward();}else if(id==kReload)v->Reload();else if(id==kHome)v->Navigate(HomeUrl().c_str());return 0;}
 case WM_KEYDOWN:if(wp==VK_F5&&active>=0&&active<static_cast<int>(tabs.size())&&tabs[active]->view){tabs[active]->view->Reload();return 0;}break;
 case WM_DESTROY:for(auto& t:tabs){if(t->controller)t->controller->Close();t->view.Reset();t->controller.Reset();}tabs.clear();environment.Reset();if(uiFont){DeleteObject(uiFont);uiFont=nullptr;}if(uiBrush){DeleteObject(uiBrush);uiBrush=nullptr;}PostQuitMessage(0);return 0;
 }return DefWindowProcW(h,msg,wp,lp);}
LRESULT CALLBACK AddressProc(HWND h,UINT msg,WPARAM wp,LPARAM lp){if(msg==WM_KEYDOWN&&wp==VK_RETURN){NavigateText();return 0;}return CallWindowProcW(oldAddressProc,h,msg,wp,lp);}
}
int WINAPI wWinMain(HINSTANCE inst,HINSTANCE,LPWSTR,int show){HRESULT init=CoInitializeEx(nullptr,COINIT_APARTMENTTHREADED);if(FAILED(init))return 1;
 uiBrush=CreateSolidBrush(kBackground);
 uiFont=CreateFontW(-17,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,DEFAULT_PITCH,L"Segoe UI");
 WNDCLASSW wc{};wc.hInstance=inst;wc.lpfnWndProc=WindowProc;wc.lpszClassName=L"MaenBrowserMain";wc.hCursor=LoadCursor(nullptr,IDC_ARROW);wc.hIcon=LoadIcon(inst,MAKEINTRESOURCE(IDI_MAENBROWSER));wc.hbrBackground=(HBRUSH)(COLOR_WINDOW+1);RegisterClassW(&wc);
 WNDCLASSW tc{};tc.hInstance=inst;tc.lpfnWndProc=TabsProc;tc.lpszClassName=L"MaenTabs";tc.hbrBackground=(HBRUSH)(COLOR_BTNFACE+1);RegisterClassW(&tc);
 windowHandle=CreateWindowW(wc.lpszClassName,L"MaenBrowser",WS_OVERLAPPEDWINDOW,CW_USEDEFAULT,CW_USEDEFAULT,1200,800,nullptr,nullptr,inst,nullptr);
 tabsBar=CreateWindowW(tc.lpszClassName,L"",WS_CHILD|WS_VISIBLE,0,0,100,kTab,windowHandle,nullptr,inst,nullptr);
 auto button=[&](const wchar_t* text,int id){CreateWindowW(L"BUTTON",text,WS_CHILD|WS_VISIBLE,0,0,40,30,windowHandle,(HMENU)(INT_PTR)id,inst,nullptr);};
 button(L"‹",kBack);button(L"›",kForward);button(L"↻",kReload);button(L"⌂ Home",kHome);button(L"+",kNew);
 for(int id:{kBack,kForward,kReload,kHome,kNew})SendMessageW(GetDlgItem(windowHandle,id),WM_SETFONT,(WPARAM)uiFont,TRUE);
 address=CreateWindowExW(WS_EX_CLIENTEDGE,L"EDIT",L"",WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL,0,0,100,32,windowHandle,(HMENU)(INT_PTR)kAddress,inst,nullptr);
 SendMessageW(address,WM_SETFONT,(WPARAM)uiFont,TRUE);
 SendMessageW(address,EM_SETMARGINS,EC_LEFTMARGIN|EC_RIGHTMARGIN,MAKELPARAM(10,10));
 oldAddressProc=(WNDPROC)SetWindowLongPtrW(address,GWLP_WNDPROC,(LONG_PTR)AddressProc);
 ShowWindow(windowHandle,show);UpdateWindow(windowHandle);Layout();
 wchar_t local[MAX_PATH]{};if(!GetEnvironmentVariableW(L"LOCALAPPDATA",local,MAX_PATH)){MessageBoxW(windowHandle,L"LOCALAPPDATA unavailable.",L"MaenBrowser",MB_ICONERROR);DestroyWindow(windowHandle);CoUninitialize();return 1;}userData=std::wstring(local)+L"\\MaenBrowser\\WebView2";
 HRESULT hr=CreateCoreWebView2EnvironmentWithOptions(nullptr,userData.c_str(),nullptr,Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
 [](HRESULT result,ICoreWebView2Environment* env)->HRESULT{if(FAILED(result)||!env){MessageBoxW(windowHandle,L"WebView2 Runtime is missing. Reinstall MaenBrowser.",L"MaenBrowser",MB_OK|MB_ICONERROR);return S_OK;}environment=env;
 int argc=0;LPWSTR* argv=CommandLineToArgvW(GetCommandLineW(),&argc);
 std::wstring initial=HomeUrl();
 if(argv){if(argc>1){std::wstring candidate=argv[1];if(candidate.rfind(L"https://",0)==0||candidate.rfind(L"http://",0)==0)initial=candidate;}LocalFree(argv);}
 AddTab(initial);return S_OK;}).Get());
 if(FAILED(hr))MessageBoxW(windowHandle,L"Could not initialize WebView2.",L"MaenBrowser",MB_OK|MB_ICONERROR);
 MSG msg{};while(GetMessageW(&msg,nullptr,0,0)>0){TranslateMessage(&msg);DispatchMessageW(&msg);}CoUninitialize();return (int)msg.wParam;
}
