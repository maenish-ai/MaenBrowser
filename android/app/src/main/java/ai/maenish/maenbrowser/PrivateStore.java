package ai.maenish.maenbrowser;

import android.content.Context;
import android.content.SharedPreferences;
import android.security.keystore.*;
import android.util.Base64;
import org.json.*;
import java.nio.charset.StandardCharsets;
import java.security.*;
import javax.crypto.*;
import javax.crypto.spec.*;

/** App-private, authenticated encrypted sessions and parental state. No cookies exported. */
final class PrivateStore {
    private final SharedPreferences prefs;
    private final SecretKey key;
    private JSONObject state;
    private String committed;
    private static volatile boolean storageWriteFailed;
    private static PrivateStore processStore;
    interface PolicyListener { void changed(boolean enabled); }
    private final java.util.concurrent.CopyOnWriteArraySet<PolicyListener> listeners=new java.util.concurrent.CopyOnWriteArraySet<>();
    static synchronized PrivateStore shared(Context context)throws Exception{
        if(processStore==null)processStore=new PrivateStore(context.getApplicationContext());
        return processStore;
    }
    void addPolicyListener(PolicyListener listener){listeners.add(listener);}
    void removePolicyListener(PolicyListener listener){listeners.remove(listener);}
    private void policyChanged(boolean enabled){
        new android.os.Handler(android.os.Looper.getMainLooper()).post(()->{for(PolicyListener listener:listeners)listener.changed(enabled);});
    }
    final boolean damaged;
    PrivateStore(Context c)throws Exception{
        prefs=c.getSharedPreferences("private-v1",Context.MODE_PRIVATE);
        KeyStore ks=KeyStore.getInstance("AndroidKeyStore");ks.load(null);
        String alias="MaenBrowser.private.v1";
        if(!ks.containsAlias(alias)){
            KeyGenerator generator=KeyGenerator.getInstance(KeyProperties.KEY_ALGORITHM_AES,"AndroidKeyStore");
            generator.init(new KeyGenParameterSpec.Builder(alias,KeyProperties.PURPOSE_ENCRYPT|KeyProperties.PURPOSE_DECRYPT)
                .setBlockModes(KeyProperties.BLOCK_MODE_GCM).setEncryptionPaddings(KeyProperties.ENCRYPTION_PADDING_NONE).build());generator.generateKey();
        }
        key=(SecretKey)ks.getKey(alias,null);
        boolean bad=false;
        try{
            String saved=prefs.getString("sealed",null);
            if(saved==null)state=new JSONObject();
            else{String[]p=saved.split(":");if(p.length!=2)throw new SecurityException();Cipher cipher=Cipher.getInstance("AES/GCM/NoPadding");cipher.init(Cipher.DECRYPT_MODE,key,new GCMParameterSpec(128,Base64.decode(p[0],Base64.NO_WRAP)));state=new JSONObject(new String(cipher.doFinal(Base64.decode(p[1],Base64.NO_WRAP)),StandardCharsets.UTF_8));}
        }catch(Exception e){bad=true;state=new JSONObject();state.put("family",true);}
        damaged=bad;
        committed=state.toString();
    }
    synchronized boolean family(){return state.optBoolean("family",false)||damaged||storageWriteFailed;}
    synchronized boolean ads(){return state.optBoolean("ads",true);}
    synchronized boolean thirdParty(){return !family()&&state.optBoolean("thirdParty",false);}
    synchronized void ads(boolean enabled)throws Exception{if(family()||damaged)throw new SecurityException("Unlock family mode first");state.put("ads",enabled);save();}
    synchronized void thirdParty(boolean enabled)throws Exception{if(family())throw new SecurityException("Unlock family mode first");state.put("thirdParty",enabled);save();}
    synchronized JSONArray sessions(){try{return new JSONArray(state.optJSONArray("tabs")==null?"[]":state.getJSONArray("tabs").toString());}catch(Exception e){return new JSONArray();}}
    synchronized void sessions(JSONArray tabs)throws Exception{
        if(damaged||tabs.length()==0)return; // Preserve last good set on startup/renderer failure.
        if(tabs.length()>12)throw new IllegalArgumentException();
        state.put("tabsPrevious",state.optJSONArray("tabs"));state.put("tabs",tabs);save();
    }
    boolean setFamily(boolean enabled,char[]pin)throws Exception{
        try{
            final String old;
            final byte[]salt;
            synchronized(this){
                if(damaged)throw new SecurityException("Protected storage is damaged; family protection remains locked.");
                if(System.currentTimeMillis()<state.optLong("retryAfter",0))return false;
                old=state.optString("pin","");
                if(old.isEmpty()){
                    if(pin.length<8)throw new IllegalArgumentException("Use a parent passphrase with at least 8 characters.");
                    salt=new byte[16];new SecureRandom().nextBytes(salt);
                }else salt=Base64.decode(state.getString("salt"),Base64.NO_WRAP);
            }
            // Expensive KDF outside the monitor: network/UI policy reads stay quick.
            byte[]derived=derive(pin,salt);
            synchronized(this){
                long now=System.currentTimeMillis();
                if(!old.equals(state.optString("pin",""))||now<state.optLong("retryAfter",0))return false;
                if(!old.isEmpty()&&!MessageDigest.isEqual(derived,Base64.decode(old,Base64.NO_WRAP))){
                    int failed=state.optInt("failures",0)+1;state.put("failures",failed);state.put("retryAfter",now+Math.min(3600000L,1000L*(1L<<Math.min(12,failed))));save();return false;
                }
                if(old.isEmpty()){state.put("salt",Base64.encodeToString(salt,Base64.NO_WRAP));state.put("pin",Base64.encodeToString(derived,Base64.NO_WRAP));}
                boolean changed=state.optBoolean("family",false)!=enabled;
                state.put("failures",0);state.put("retryAfter",0);state.put("family",enabled);save();if(changed)policyChanged(enabled);return true;
            }
        }finally{java.util.Arrays.fill(pin,'\0');}
    }
    private byte[] derive(char[]pin,byte[]salt)throws Exception{
        PBEKeySpec spec=new PBEKeySpec(pin,salt,210000,256);
        try{return SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256").generateSecret(spec).getEncoded();}finally{spec.clearPassword();}
    }
    private void save()throws Exception{
        String next=state.toString();
        try{
            Cipher cipher=Cipher.getInstance("AES/GCM/NoPadding");cipher.init(Cipher.ENCRYPT_MODE,key);
            String value=Base64.encodeToString(cipher.getIV(),Base64.NO_WRAP)+":"+Base64.encodeToString(cipher.doFinal(next.getBytes(StandardCharsets.UTF_8)),Base64.NO_WRAP);
            if(!prefs.edit().putString("sealed",value).commit())throw new java.io.IOException("Private storage write failed");
            committed=next;
        }catch(Exception failure){
            // A failed disable/write must never relax the in-memory parent policy.
            storageWriteFailed=true;state=new JSONObject(committed);policyChanged(true);throw failure;
        }
    }
}
