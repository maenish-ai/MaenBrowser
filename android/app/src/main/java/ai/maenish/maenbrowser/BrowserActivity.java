package ai.maenish.maenbrowser;

import android.Manifest;
import android.app.*;
import android.content.*;
import android.content.pm.PackageManager;
import android.graphics.Bitmap;
import android.net.Uri;
import android.net.http.SslError;
import android.os.*;
import android.view.*;
import android.view.inputmethod.EditorInfo;
import android.webkit.*;
import android.widget.*;
import org.json.JSONArray;
import java.io.ByteArrayInputStream;
import java.util.*;

/** Native UI; no JS/native bridge, tracking SDK, background service, or bundled engine. */
public final class BrowserActivity extends Activity {
    private final ArrayList<Tab> tabs=new ArrayList<>();
    private static final java.util.concurrent.ExecutorService persistence=new java.util.concurrent.ThreadPoolExecutor(0,1,30L,java.util.concurrent.TimeUnit.SECONDS,new java.util.concurrent.LinkedBlockingQueue<Runnable>());
    private final Handler ui=new Handler(Looper.getMainLooper());
    private FrameLayout content;
    private EditText address;
    private ProgressBar progress;
    private TextView status;
    private PrivateStore store;
    private final PrivateStore.PolicyListener policyListener=this::applyFamilyPolicy;
    private DomainFilters filters;
    private int selected=-1, budget=4;
    private ValueCallback<Uri[]> fileCallback;
    private WebView fileOwner;
    private String fileOrigin="";
    private PermissionRequest mediaRequest;
    private WebChromeClient fullscreenOwner;
    private boolean destroyed;
    // Process-local rotation handoff only. Durable sessions remain encrypted.
    private static volatile String rotationTabs;
    private final Runnable saveTask=()->saveSession();
    private static final class Tab { WebView view; String url; Tab(String u){url=u;} }

    @Override public void onCreate(Bundle saved){
        super.onCreate(saved);
        try{store=PrivateStore.shared(this);}catch(Exception e){new AlertDialog.Builder(this).setTitle("MaenBrowser").setMessage("Secure storage unavailable. Browsing is disabled.").setPositiveButton("Close",(d,w)->finish()).setCancelable(false).show();return;}
        filters=new DomainFilters(getApplicationContext());
        store.addPolicyListener(policyListener);
        if(store.family())filters.loadFamily(getApplicationContext());
        ActivityManager am=(ActivityManager)getSystemService(ACTIVITY_SERVICE);
        ActivityManager.MemoryInfo memory=new ActivityManager.MemoryInfo();am.getMemoryInfo(memory);
        budget=am.isLowRamDevice()||memory.totalMem<=3L*1024*1024*1024?2:memory.totalMem>=8L*1024*1024*1024?6:4;
        WebView.setWebContentsDebuggingEnabled(BuildConfig.DEBUG);
        CookieManager.getInstance().setAcceptCookie(true);
        WebView.startSafeBrowsing(getApplicationContext(),ok->{if(!ok)notice("System Safe Browsing is unavailable; keep WebView updated.");});
        buildUi();
        if(Build.VERSION.SDK_INT>=33)getOnBackInvokedDispatcher().registerOnBackInvokedCallback(android.window.OnBackInvokedDispatcher.PRIORITY_DEFAULT,this::handleBack);
        getWindow().getDecorView().setOnApplyWindowInsetsListener((v,insets)->{if(Build.VERSION.SDK_INT>=30){android.graphics.Insets i=insets.getInsets(WindowInsets.Type.systemBars()|WindowInsets.Type.ime());v.setPadding(i.left,i.top,i.right,i.bottom);}else{v.setPadding(insets.getSystemWindowInsetLeft(),insets.getSystemWindowInsetTop(),insets.getSystemWindowInsetRight(),insets.getSystemWindowInsetBottom());}return insets;});
        JSONArray savedTabs=store.sessions();
        if(saved!=null && rotationTabs!=null){try{savedTabs=new JSONArray(rotationTabs);}catch(org.json.JSONException ignored){}}

        for(int i=0;i<Math.min(savedTabs.length(),budget);i++){String url=WebPolicy.address(savedTabs.optString(i));if(!url.isEmpty())tabs.add(new Tab(url));}
        String incoming=saved==null?incoming(getIntent()):"";
        int initial=0;
        if(!incoming.isEmpty()){
            if(tabs.size()<budget){tabs.add(new Tab(incoming));initial=tabs.size()-1;}
            else notice("Tab limit reached. Close a tab, then open the external link again. Existing tabs were preserved.");
        }
        if(tabs.isEmpty())tabs.add(new Tab("https://www.google.com/"));
        if(saved!=null)initial=Math.max(0,Math.min(saved.getInt("selectedTab",0),tabs.size()-1));
        select(initial);
        if(store.damaged)notice("Family protection locked: encrypted settings are damaged. No automatic reset.");
    }
    private void buildUi(){
        LinearLayout root=new LinearLayout(this);root.setOrientation(LinearLayout.VERTICAL);
        LinearLayout bar=new LinearLayout(this);
        Button back=button("‹",()->{WebView v=current();if(v!=null&&v.canGoBack())v.goBack();});bar.addView(back,new LinearLayout.LayoutParams(dp(48),dp(52)));
        address=new EditText(this);address.setSingleLine(true);address.setInputType(android.text.InputType.TYPE_CLASS_TEXT|android.text.InputType.TYPE_TEXT_VARIATION_URI);address.setImeOptions(EditorInfo.IME_ACTION_GO);address.setTextSize(14);address.setContentDescription(getString(R.string.address));
        address.setOnEditorActionListener((v,action,event)->{if(action==EditorInfo.IME_ACTION_GO){navigate(address.getText().toString());return true;}return false;});
        bar.addView(address,new LinearLayout.LayoutParams(0,dp(52),1));bar.addView(button("↻",()->{WebView v=current();if(v!=null){String u=v.getUrl();if(u==null||u.equals("about:blank"))navigate(tabs.get(selected).url);else v.reload();}}),new LinearLayout.LayoutParams(dp(48),dp(52)));bar.addView(button("☰",this::menu),new LinearLayout.LayoutParams(dp(48),dp(52)));root.addView(bar);
        progress=new ProgressBar(this,null,android.R.attr.progressBarStyleHorizontal);root.addView(progress,new LinearLayout.LayoutParams(-1,dp(3)));
        status=new TextView(this);status.setTextSize(12);status.setPadding(dp(8),dp(3),dp(8),dp(3));status.setText("MaenBrowser · HTTPS · system WebView");root.addView(status);
        content=new FrameLayout(this);root.addView(content,new LinearLayout.LayoutParams(-1,0,1));setContentView(root);
    }
    private int dp(int value){return Math.round(value*getResources().getDisplayMetrics().density);}
    private Button button(String title,Runnable action){Button b=new Button(this);b.setText(title);b.setPadding(0,0,0,0);b.setOnClickListener(v->action.run());return b;}
    private void notice(String text){ui.post(()->{if(!destroyed){if(status!=null)status.setText(text);Toast.makeText(this,text,Toast.LENGTH_LONG).show();}});}
    private WebView current(){return selected>=0&&selected<tabs.size()?tabs.get(selected).view:null;}
    private boolean blocked(String url,boolean main){
        if(WebPolicy.host(url).isEmpty())return true;
        return filters.familyBlocked(url,store.family())||filters.adsBlocked(url,store.ads());
    }
    private void navigate(String raw){
        String url=WebPolicy.address(raw);if(url.isEmpty()){notice("Enter a valid HTTPS website address.");return;}
        if(blocked(url,true)){notice("Blocked by protection, or family lists are still loading. Try again when ready.");return;}
        Tab t=tabs.get(selected);t.url=url;current().loadUrl(url);
    }
    private WebView create(Tab tab){
        WebView v=new WebView(this);WebSettings s=v.getSettings();
        s.setJavaScriptEnabled(true);s.setDomStorageEnabled(true);s.setAllowFileAccess(false);s.setAllowContentAccess(false);
        s.setMixedContentMode(WebSettings.MIXED_CONTENT_NEVER_ALLOW);s.setSafeBrowsingEnabled(true);
        s.setMediaPlaybackRequiresUserGesture(true);s.setSupportMultipleWindows(true);s.setJavaScriptCanOpenWindowsAutomatically(false);
        s.setBuiltInZoomControls(true);s.setDisplayZoomControls(false);s.setUseWideViewPort(true);s.setLoadWithOverviewMode(true);
        CookieManager.getInstance().setAcceptThirdPartyCookies(v,store.thirdParty());
        v.setWebViewClient(new WebViewClient(){
            @Override public boolean shouldOverrideUrlLoading(WebView view,WebResourceRequest r){
                String raw=r.getUrl().toString();
                if(!r.isForMainFrame())return !raw.startsWith("https://");
                String safe=WebPolicy.address(raw);
                if(safe.isEmpty()||blocked(safe,true)){notice("Navigation blocked by HTTPS or family protection.");return true;}
                if(!safe.equals(raw)){view.loadUrl(safe);return true;}return false;
            }
            @Override public WebResourceResponse shouldInterceptRequest(WebView view,WebResourceRequest r){
                String u=r.getUrl().toString();
                // Local about:blank and renderer-owned blob/data subresources remain
                // available for media/editors; external navigation never accepts them.
                if(u.startsWith("https://"))return blocked(u,r.isForMainFrame())?denied():null;
                if(!r.isForMainFrame()&&(u.startsWith("blob:")||u.startsWith("data:")))return null;
                return denied();
            }
            @Override public void onReceivedSslError(WebView view,SslErrorHandler handler,SslError error){handler.cancel();notice("TLS certificate rejected. Connection cancelled.");}
            @Override public void onSafeBrowsingHit(WebView view,WebResourceRequest request,int threat,SafeBrowsingResponse response){response.backToSafety(true);notice("Unsafe website blocked by system Safe Browsing.");}
            @Override public void onPageStarted(WebView view,String url,Bitmap icon){if(view==current()){address.setText(url);status.setText("MaenBrowser · loading HTTPS");}}
            @Override public void onPageFinished(WebView view,String url){
                if(!WebPolicy.host(url).isEmpty()&&!blocked(url,true)){tab.url=url;ui.removeCallbacks(saveTask);ui.postDelayed(saveTask,600);}
                if(view==current())status.setText("MaenBrowser · "+(store.family()?"family protection · ":"")+(filters.ready(store.family())?"local filters ready":"filters loading / unavailable"));
            }
            @Override public boolean onRenderProcessGone(WebView view,RenderProcessGoneDetail detail){
                boolean active=view==current();if(active)content.removeAllViews();view.destroy();tab.view=null;
                if(active){TextView retry=new TextView(BrowserActivity.this);retry.setText("The page process stopped. Tap to reload this tab.");retry.setPadding(dp(24),dp(48),dp(24),dp(48));retry.setOnClickListener(w->select(tabs.indexOf(tab)));content.addView(retry);}return true;
            }
        });
        v.setWebChromeClient(new WebChromeClient(){
            private View fullscreen;private CustomViewCallback fullscreenCallback;
            @Override public void onProgressChanged(WebView view,int p){if(view==current()){progress.setProgress(p);progress.setVisibility(p==100?View.GONE:View.VISIBLE);}}
            @Override public void onPermissionRequest(PermissionRequest request){ui.post(()->requestMedia(request));}
            @Override public void onPermissionRequestCanceled(PermissionRequest request){if(mediaRequest==request){request.deny();mediaRequest=null;}}
            @Override public void onGeolocationPermissionsShowPrompt(String origin,GeolocationPermissions.Callback callback){callback.invoke(origin,false,false);}
            @Override public boolean onShowFileChooser(WebView view,ValueCallback<Uri[]> callback,FileChooserParams params){
                if(store.family()||view!=current()||WebPolicy.origin(view.getUrl()).isEmpty()){callback.onReceiveValue(null);notice("File access is unavailable for this page.");return true;}
                if(fileCallback!=null)fileCallback.onReceiveValue(null);fileCallback=callback;fileOwner=view;fileOrigin=WebPolicy.origin(view.getUrl());
                Intent pick=new Intent(Intent.ACTION_OPEN_DOCUMENT).setType("*/*").addCategory(Intent.CATEGORY_OPENABLE);
                pick.putExtra(Intent.EXTRA_ALLOW_MULTIPLE,params.getMode()==FileChooserParams.MODE_OPEN_MULTIPLE);
                try{startActivityForResult(pick,70);}catch(Exception e){fileCallback.onReceiveValue(null);fileCallback=null;}return true;
            }
            @Override public boolean onCreateWindow(WebView view,boolean dialog,boolean gesture,Message message){
                if(!gesture||store.family()){notice("Popup blocked.");return false;}
                if(tabs.size()>=budget){notice("Tab limit reached. Close a tab before opening another.");return false;}
                Tab popup=new Tab("https://www.google.com/");popup.view=create(popup);tabs.add(popup);select(tabs.size()-1);
                ((WebView.WebViewTransport)message.obj).setWebView(popup.view);message.sendToTarget();return true;
            }
            @Override public void onCloseWindow(WebView window){for(int i=0;i<tabs.size();i++)if(tabs.get(i).view==window){closeTab(i);return;}}
            @Override public void onShowCustomView(View view,CustomViewCallback cb){if(fullscreen!=null){cb.onCustomViewHidden();return;}fullscreen=view;fullscreenCallback=cb;fullscreenOwner=this;addContentView(view,new ViewGroup.LayoutParams(-1,-1));getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);}
            @Override public void onHideCustomView(){if(fullscreen!=null){((ViewGroup)fullscreen.getParent()).removeView(fullscreen);fullscreen=null;fullscreenOwner=null;getWindow().clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);fullscreenCallback.onCustomViewHidden();fullscreenCallback=null;}}
        });
        v.setDownloadListener((url,ua,disposition,type,length)->{
            if(WebPolicy.host(url).isEmpty()||blocked(url,true)||store.family()){notice("Download blocked. Protected, blob and non-HTTPS downloads are unavailable.");return;}
            new AlertDialog.Builder(this).setTitle("Download file?").setMessage(WebPolicy.host(url)+"\nFiles are never opened automatically. Login-protected downloads may need a website download link.").setNegativeButton(android.R.string.cancel,null).setPositiveButton("Download",(d,w)->{
                try{android.app.DownloadManager.Request request=new android.app.DownloadManager.Request(Uri.parse(url));request.setTitle("MaenBrowser download");request.setNotificationVisibility(android.app.DownloadManager.Request.VISIBILITY_VISIBLE_NOTIFY_COMPLETED);request.setDestinationInExternalPublicDir(Environment.DIRECTORY_DOWNLOADS,safeFileName(url,disposition,type));((android.app.DownloadManager)getSystemService(DOWNLOAD_SERVICE)).enqueue(request);}
                catch(Exception e){notice("Download could not be started.");}
            }).show();
        });return v;
    }
    private static String safeFileName(String url,String disposition,String type){String name=URLUtil.guessFileName(url,disposition,type).replaceAll("[^A-Za-z0-9._-]","_");return UUID.randomUUID()+"-"+name.substring(0,Math.min(100,name.length()));}
    private static WebResourceResponse denied(){return new WebResourceResponse("text/plain","UTF-8",403,"Blocked",Collections.singletonMap("Cache-Control","no-store"),new ByteArrayInputStream(new byte[0]));}
    private void select(int index){
        if(index<0||index>=tabs.size())return;
        if(fullscreenOwner!=null)fullscreenOwner.onHideCustomView();
        WebView previous=current();if(previous!=null)previous.onPause();content.removeAllViews();selected=index;Tab t=tabs.get(index);
        boolean fresh=t.view==null;if(fresh)t.view=create(t);
        if(t.view.getParent()!=null)((ViewGroup)t.view.getParent()).removeView(t.view);
        content.addView(t.view,new FrameLayout.LayoutParams(-1,-1));t.view.onResume();address.setText(t.url);
        if(fresh){if(blocked(t.url,true))notice("Page kept unloaded until protection lists are ready. Press reload or Go.");else t.view.loadUrl(t.url);}
    }
    private void closeTab(int index){
        if(index<0||index>=tabs.size())return;
        if(fullscreenOwner!=null)fullscreenOwner.onHideCustomView();
        Tab t=tabs.remove(index);if(t.view!=null){if(t.view.getParent()!=null)((ViewGroup)t.view.getParent()).removeView(t.view);t.view.destroy();}
        selected=-1;if(tabs.isEmpty())tabs.add(new Tab("https://www.google.com/"));select(Math.min(index,tabs.size()-1));saveSession();
    }
    private void menu(){
        String[]options={getString(R.string.tabs_count,tabs.size(),budget),getString(R.string.new_tab),getString(R.string.close_tab),getString(R.string.settings),getString(R.string.about)};
        new AlertDialog.Builder(this).setTitle("MaenBrowser").setItems(options,(d,w)->{switch(w){
            case 0->{String[]names=tabs.stream().map(t->t.view!=null&&t.view.getTitle()!=null?t.view.getTitle():t.url).toArray(String[]::new);new AlertDialog.Builder(this).setTitle(R.string.tabs_lazy).setItems(names,(a,i)->select(i)).show();}
            case 1->{if(tabs.size()>=budget)notice("Tab limit protects memory. Close an existing tab first.");else{tabs.add(new Tab("https://www.google.com/"));select(tabs.size()-1);}}
            case 2->closeTab(selected);
            case 3->settings();
            case 4->{PackageInfoText();}
        }}).show();
    }
    private void PackageInfoText(){
        android.content.pm.PackageInfo provider=WebView.getCurrentWebViewPackage();
        new AlertDialog.Builder(this).setTitle("MaenBrowser "+BuildConfig.VERSION_NAME).setMessage("Native Android preview · Android 10+\nSystem WebView: "+(provider==null?"unavailable":provider.packageName+" "+provider.versionName)+"\n\nUpdate Android System WebView through Play Store for engine security and codecs. MP4/H.264/AAC and other formats depend on the installed engine/device. No desktop extensions or Flash. No guarantee that cloud Agents support an external browser.\n\nTab budget: "+budget+". Only addresses are restored, not unsaved forms. No Maen analytics.").setPositiveButton(android.R.string.ok,null).show();
    }
    private void settings(){
        String[]options={getString(store.ads()?R.string.ads_off:R.string.ads_on),getString(store.family()?R.string.family_off:R.string.family_on),getString(store.thirdParty()?R.string.cookies_off:R.string.cookies_on),getString(R.string.clear_data)};
        new AlertDialog.Builder(this).setTitle(R.string.settings).setItems(options,(d,w)->{try{
            if(w==0){store.ads(!store.ads());notice("Ad filtering changed. Reload the page. Detection is still possible.");}
            if(w==1){parentPrompt();}
            if(w==2){store.thirdParty(!store.thirdParty());for(Tab t:tabs)if(t.view!=null)CookieManager.getInstance().setAcceptThirdPartyCookies(t.view,store.thirdParty());notice("Cookie compatibility changed. Reload the page.");}
            if(w==3){if(store.family())throw new SecurityException("Unlock family protection first.");new AlertDialog.Builder(this).setTitle("Clear cookies and site storage?").setNegativeButton(android.R.string.cancel,null).setPositiveButton("Clear",(x,y)->{CookieManager.getInstance().removeAllCookies(null);WebStorage.getInstance().deleteAllData();for(Tab t:tabs)if(t.view!=null){t.view.clearCache(true);t.view.clearHistory();}notice("Browsing data cleared.");}).show();}
        }catch(Exception e){notice(e.getMessage()==null?"Settings could not be saved.":e.getMessage());}}).show();
    }
    private void applyFamilyPolicy(boolean enabled){
        if(destroyed || !enabled)return;
        filters.loadFamily(getApplicationContext());denyMedia();
        if(fileCallback!=null){fileCallback.onReceiveValue(null);fileCallback=null;fileOwner=null;fileOrigin="";}
        if(fullscreenOwner!=null)fullscreenOwner.onHideCustomView();
        for(Tab tab:tabs)if(tab.view!=null){tab.view.stopLoading();tab.view.loadUrl("about:blank");CookieManager.getInstance().setAcceptThirdPartyCookies(tab.view,false);}
    }
    private void parentPrompt(){
        EditText pin=new EditText(this);pin.setInputType(129);pin.setSingleLine(true);
        new AlertDialog.Builder(this).setTitle(store.family()?"Parent passphrase":"Set / enter parent passphrase (8+ characters)").setView(pin).setNegativeButton(android.R.string.cancel,null).setPositiveButton("Apply",(d,w)->{
            char[]secret=pin.getText().toString().toCharArray();pin.setText("");
            // KDF and synchronous durable write never run on the UI thread.
            new Thread(()->{try{boolean on=!store.family();boolean ok=store.setFamily(on,secret);ui.post(()->{if(destroyed)return;if(!ok){notice("Passphrase rejected or retry delay active.");return;}notice(store.family()?"Family protection enabled. Reload pages after lists load.":"Family protection disabled.");});}catch(Exception e){notice(e.getMessage()==null?"Protected setting failed.":e.getMessage());}},"Maen-parent-policy").start();
        }).show();
    }
    private void requestMedia(PermissionRequest request){
        if(destroyed){request.deny();return;}
        WebView v=current();String origin=WebPolicy.origin(request.getOrigin().toString());
        if(v==null||store.family()||origin.isEmpty()||!origin.equals(WebPolicy.origin(v.getUrl()))||mediaRequest!=null){request.deny();return;}
        ArrayList<String>permissions=new ArrayList<>();
        for(String r:request.getResources()){
            if(r.equals(PermissionRequest.RESOURCE_VIDEO_CAPTURE))permissions.add(Manifest.permission.CAMERA);
            else if(r.equals(PermissionRequest.RESOURCE_AUDIO_CAPTURE))permissions.add(Manifest.permission.RECORD_AUDIO);
            else{request.deny();return;}
        }
        mediaRequest=request;
        new AlertDialog.Builder(this).setTitle("Camera / microphone permission").setMessage(origin+" requests "+String.join(", ",permissions)+" for this session.").setNegativeButton("Deny",(d,w)->denyMedia()).setOnCancelListener(d->denyMedia()).setPositiveButton("Allow",(d,w)->{
            if(mediaRequest!=request)return;
            ArrayList<String>missing=new ArrayList<>();for(String p:permissions)if(checkSelfPermission(p)!=PackageManager.PERMISSION_GRANTED)missing.add(p);
            if(missing.isEmpty())grantMedia();else requestPermissions(missing.toArray(new String[0]),71);
        }).show();
    }
    private void denyMedia(){if(mediaRequest!=null){mediaRequest.deny();mediaRequest=null;}}
    private void grantMedia(){
        PermissionRequest r=mediaRequest;mediaRequest=null;WebView v=current();if(r==null)return;
        if(v==null||store.family()||!WebPolicy.origin(r.getOrigin().toString()).equals(WebPolicy.origin(v.getUrl()))){r.deny();return;}
        for(String s:r.getResources()){String p=s.equals(PermissionRequest.RESOURCE_VIDEO_CAPTURE)?Manifest.permission.CAMERA:Manifest.permission.RECORD_AUDIO;if(checkSelfPermission(p)!=PackageManager.PERMISSION_GRANTED){r.deny();return;}}
        r.grant(r.getResources());
    }
    @Override public void onRequestPermissionsResult(int code,String[]permissions,int[]results){super.onRequestPermissionsResult(code,permissions,results);if(code==71)grantMedia();}
    @Override protected void onActivityResult(int code,int result,Intent data){super.onActivityResult(code,result,data);if(code==70&&fileCallback!=null){ValueCallback<Uri[]>c=fileCallback;fileCallback=null;ArrayList<Uri>uris=new ArrayList<>();if(result==RESULT_OK&&data!=null&&!store.family()&&current()==fileOwner&&fileOwner!=null&&fileOrigin.equals(WebPolicy.origin(fileOwner.getUrl()))){if(data.getClipData()!=null){for(int i=0;i<Math.min(data.getClipData().getItemCount(),20);i++){Uri u=data.getClipData().getItemAt(i).getUri();if("content".equals(u.getScheme()))uris.add(u);}}else if(data.getData()!=null&&"content".equals(data.getData().getScheme()))uris.add(data.getData());}fileOwner=null;fileOrigin="";c.onReceiveValue(uris.isEmpty()?null:uris.toArray(new Uri[0]));}}
    private String incoming(Intent i){return Intent.ACTION_VIEW.equals(i.getAction())&&i.getData()!=null?WebPolicy.address(i.getData().toString()):"";}
    @Override protected void onNewIntent(Intent intent){
        super.onNewIntent(intent);setIntent(intent);String u=incoming(intent);
        if(u.isEmpty()||current()==null)return;
        // Keep the current chat/draft and its request context intact.
        if(tabs.size()>=budget){notice("Tab limit reached. Close a tab, then open the external link again. Existing tabs were preserved.");return;}
        if(blocked(u,true)){notice("External link blocked by protection, or family lists are still loading.");return;}
        tabs.add(new Tab(u));select(tabs.size()-1);saveSession();
    }
    private JSONArray sessionAddresses(){JSONArray a=new JSONArray();for(Tab t:tabs)if(!WebPolicy.host(t.url).isEmpty())a.put(t.url);return a;}
    @Override protected void onSaveInstanceState(Bundle out){rotationTabs=sessionAddresses().toString();out.putInt("selectedTab",selected);super.onSaveInstanceState(out);}
    private void saveSession(){if(store==null||store.damaged)return;JSONArray a=sessionAddresses();rotationTabs=a.toString();if(!persistence.isShutdown())persistence.execute(()->{try{store.sessions(a);CookieManager.getInstance().flush();}catch(Exception e){notice("Session could not be saved; previous session retained.");}});}
    @Override protected void onPause(){super.onPause();if(current()!=null)current().onPause();saveSession();}
    @Override protected void onResume(){super.onResume();if(current()!=null)current().onResume();}
    private void handleBack(){if(fullscreenOwner!=null){fullscreenOwner.onHideCustomView();return;}WebView v=current();if(v!=null&&v.canGoBack())v.goBack();else finish();}
    // API29–32 fallback only. API33+ uses the platform callback registered above,
    // including Android16 predictive back. No AndroidX runtime dependency needed.
    @android.annotation.SuppressLint("GestureBackNavigation")
    @Override public void onBackPressed(){handleBack();}
    @Override protected void onDestroy(){destroyed=true;ui.removeCallbacksAndMessages(null);denyMedia();if(fileCallback!=null)fileCallback.onReceiveValue(null);for(Tab t:tabs)if(t.view!=null){if(t.view.getParent()!=null)((ViewGroup)t.view.getParent()).removeView(t.view);t.view.destroy();}if(filters!=null)filters.close();if(store!=null)store.removePolicyListener(policyListener);super.onDestroy();}
}
