package ai.maenish.maenbrowser;

import android.app.*;
import android.content.Intent;
import android.os.Bundle;
import android.view.*;
import android.webkit.*;

/** Device/emulator checks: real Activity lifecycle and WebView security settings. */
public final class SmokeInstrumentation extends Instrumentation {
    private Activity activity;
    private WebView web;
    @Override public void onCreate(Bundle args){super.onCreate(args);start();}
    private WebView find(View v){if(v instanceof WebView)return (WebView)v;if(v instanceof ViewGroup){ViewGroup g=(ViewGroup)v;for(int i=0;i<g.getChildCount();i++){WebView w=find(g.getChildAt(i));if(w!=null)return w;}}return null;}
    private void require(boolean value,String message){if(!value)throw new AssertionError(message);}
    @Override public void onStart(){
        Bundle report=new Bundle();
        report.putString("class",SmokeInstrumentation.class.getName());report.putString("test","startupAndSecuritySettings");report.putInt("numtests",1);report.putInt("current",1);sendStatus(1,report);
        try{
            android.content.Context isolated=new android.content.ContextWrapper(getTargetContext()){
                @Override public android.content.SharedPreferences getSharedPreferences(String name,int mode){return super.getSharedPreferences("instrumentation-"+name,mode);}
            };
            isolated.getSharedPreferences("private-v1",0).edit().clear().commit();
            try{
                PrivateStore protectedStore=new PrivateStore(isolated);
                require(protectedStore.setFamily(true,"a-long-test-passphrase".toCharArray()),"family enable failed");
                require(new PrivateStore(isolated).family(),"family not durable");
                require(!protectedStore.setFamily(false,"incorrect-passphrase".toCharArray()),"wrong PIN accepted");
                require(protectedStore.family(),"wrong PIN removed protection");
                require(!protectedStore.setFamily(false,"a-long-test-passphrase".toCharArray()),"rate limit bypassed");
                String sealed=isolated.getSharedPreferences("private-v1",0).getString("sealed","");
                require(!sealed.contains("family")&&!sealed.contains("a-long-test-passphrase"),"plaintext settings persisted");
                isolated.getSharedPreferences("private-v1",0).edit().putString("sealed","tampered").commit();
                PrivateStore damaged=new PrivateStore(isolated);require(damaged.damaged&&damaged.family(),"tamper did not fail closed");
            }finally{isolated.getSharedPreferences("private-v1",0).edit().clear().commit();}
            Intent intent=new Intent().setClassName(getTargetContext().getPackageName(),BrowserActivity.class.getName()).addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            activity=startActivitySync(intent);waitForIdleSync();
            runOnMainSync(()->{
                web=find(activity.getWindow().getDecorView());require(web!=null,"WebView missing");
                WebSettings s=web.getSettings();require(!s.getAllowFileAccess(),"file access enabled");require(!s.getAllowContentAccess(),"content access enabled");require(s.getMixedContentMode()==WebSettings.MIXED_CONTENT_NEVER_ALLOW,"mixed content enabled");require(s.getSafeBrowsingEnabled(),"Safe Browsing disabled");require(s.getMediaPlaybackRequiresUserGesture(),"autoplay unexpectedly allowed");require(!CookieManager.getInstance().acceptThirdPartyCookies(web),"third-party cookies enabled by default");
                require(!WebPolicy.address("file:///sdcard/test.html").equals("file:///sdcard/test.html"),"unsafe navigation");
                activity.recreate();
            });
            waitForIdleSync();report.putString("stream","PASS: Keystore encryption, parental PIN/rate limit/tamper, Activity startup/recreation, HTTPS policy, WebView file/mixed-content/Safe-Browsing/gesture/cookie settings\n");sendStatus(0,report);finish(Activity.RESULT_OK,report);
        }catch(Throwable e){report.putString("stream","FAIL: "+e);report.putString("stack",android.util.Log.getStackTraceString(e));sendStatus(-2,report);finish(Activity.RESULT_OK,report);}
    }
}
