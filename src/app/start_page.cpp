#include "src/app/start_page.h"
#include <string>
#include "include/cef_parser.h"

namespace maenbrowser::ui {
std::string GetStartPageDataUrl() {
  static const char kHtml[] = R"HTML(<!doctype html><html><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1"><title>MaenBrowser</title>
<style>
:root{color-scheme:light dark}*{box-sizing:border-box}body{margin:0;font-family:Segoe UI,Arial,sans-serif;background:#f5f7fb;color:#172033;min-height:100vh}
main{width:min(820px,92vw);margin:0 auto;padding:11vh 0 40px}.brand{display:flex;align-items:center;justify-content:center;gap:12px;font-size:38px;font-weight:760}.m{display:grid;place-items:center;width:52px;height:52px;border-radius:15px;background:linear-gradient(135deg,#273ee8,#7955e8);color:#fff;font-weight:900}
.tag{text-align:center;color:#667085;margin:10px 0 28px}.search{display:flex;background:#fff;border:1px solid #dce2ed;border-radius:16px;padding:7px;box-shadow:0 8px 28px #1c2b4a14}
.search input{flex:1;border:0;outline:0;background:transparent;color:inherit;padding:13px;font-size:16px}.search button{border:0;border-radius:11px;padding:0 20px;background:#3146df;color:#fff;font-weight:650}
.row{display:grid;grid-template-columns:repeat(4,1fr);gap:10px;margin-top:18px}.row a,.card{background:#fff;border:1px solid #e0e5ee;border-radius:14px;padding:15px;text-decoration:none;color:inherit}
.row a{text-align:center;font-weight:600}.card{margin-top:14px}.card b{display:block;margin-bottom:6px}.small{font-size:12px;color:#737d91}.chips{display:flex;gap:7px;flex-wrap:wrap;margin-top:9px}.chip{border:1px solid #d9dfeb;border-radius:999px;padding:7px 10px;font-size:12px;background:transparent;color:inherit}
footer{text-align:center;color:#9299a8;font-size:11px;margin-top:24px}
@media(max-width:580px){.row{grid-template-columns:repeat(2,1fr)}.brand{font-size:31px}}
@media(prefers-color-scheme:dark){body{background:#111522;color:#f4f6fb}.search,.row a,.card{background:#1a2030;border-color:#30394d}.tag,.small{color:#a8b0c1}.chip{border-color:#394359}}
</style></head><body><main>
<div class="brand"><span class="m">M</span>MaenBrowser</div><div class="tag">Fast · Private · Lightweight</div>
<form class="search" action="https://www.google.com/search" method="get"><input name="q" autofocus autocomplete="off" placeholder="Search the web or type in the address bar"><button>Search</button></form>
<div class="row"><a href="https://www.youtube.com/">YouTube</a><a href="https://web.whatsapp.com/">WhatsApp</a><a href="https://mail.google.com/">Gmail</a><a href="https://www.wikipedia.org/">Wikipedia</a></div>
<div class="card"><b>Quick commands</b><div class="small">The browser keeps advanced tools out of the way. Use familiar Chromium shortcuts:</div>
<div class="chips"><span class="chip">Ctrl+L Address</span><span class="chip">Ctrl+T New tab</span><span class="chip">Ctrl+Shift+T Restore tab</span><span class="chip">Ctrl+H History</span><span class="chip">Ctrl+J Downloads</span><span class="chip">Ctrl+Shift+N Private</span></div></div>
<div class="card"><b>Performance first</b><div class="small">MaenBrowser automatically selects Lite, Balanced or Performance policy from installed RAM. No security boundary is disabled to save memory.</div></div>
<footer>MaenBrowser 1.3 · Local-first · No mandatory Maen account</footer>
</main></body></html>)HTML";
  return "data:text/html;charset=utf-8," + CefURIEncode(kHtml, false).ToString();
}
}  // namespace maenbrowser::ui
