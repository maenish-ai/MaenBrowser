package ai.maenish.maenbrowser;

import android.content.Context;
import java.io.*;
import java.nio.charset.StandardCharsets;
import java.util.*;
import java.util.concurrent.*;

/** Single background load, immutable snapshots; no per-request I/O. Family fails closed while loading. */
final class DomainFilters {
    private volatile Set<String> ads=Collections.emptySet(), family=Collections.emptySet();
    private volatile boolean adsReady, familyReady;
    private final ExecutorService loader=Executors.newSingleThreadExecutor();
    DomainFilters(Context context){
        loader.execute(()->{try{Set<String>s=read(context,"ads.txt");s.addAll(read(context,"ads-extra.txt"));ads=Collections.unmodifiableSet(s);adsReady=true;}catch(IOException ignored){}});
    }
    synchronized void loadFamily(Context c){
        if(familyReady)return;
        loader.execute(()->{try{Set<String>s=read(c,"adult.txt");s.addAll(read(c,"violence.txt"));family=Collections.unmodifiableSet(s);familyReady=true;}catch(IOException ignored){}});
    }
    boolean familyBlocked(String url, boolean enabled){return enabled&&(!familyReady||WebPolicy.matches(family,WebPolicy.host(url)));}
    boolean adsBlocked(String url,boolean enabled){return enabled&&adsReady&&WebPolicy.matches(ads,WebPolicy.host(url));}
    boolean ready(boolean familyMode){return adsReady&&(!familyMode||familyReady);}
    private Set<String> read(Context c,String file)throws IOException{
        Set<String>s=new HashSet<>();
        try(BufferedReader r=new BufferedReader(new InputStreamReader(c.getAssets().open("filters/"+file),StandardCharsets.UTF_8))){String line;while((line=r.readLine())!=null){line=line.trim();if(!line.isEmpty()&&!line.startsWith("#"))s.add(line);}}
        return s;
    }
    void close(){loader.shutdown();}
}
