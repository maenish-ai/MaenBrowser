package ai.maenish.maenbrowser;

import java.net.URI;
import java.net.IDN;
import java.util.Locale;
import java.util.Set;

/** Pure policy: no network, no Android objects, exact domain boundaries. */
final class WebPolicy {
    static String host(String input) {
        try {
            URI u = new URI(input);
            if (!"https".equalsIgnoreCase(u.getScheme()) || u.getRawUserInfo() != null) return "";
            String h = u.getHost();
            if (h == null || h.isEmpty()) return "";
            h = h.toLowerCase(Locale.ROOT);
            if (h.endsWith(".")) h=h.substring(0,h.length()-1);
            return IDN.toASCII(h);
        } catch (Exception e) { return ""; }
    }
    static String address(String input) {
        String s=input.trim();
        if (s.length()>8192 || s.isEmpty()) return "";
        if (!s.contains("://")) {
            if(s.contains(":") || s.contains(" ")) return "";
            s="https://"+s;
        }
        if (s.regionMatches(true,0,"http://",0,7)) s="https://"+s.substring(7);
        return host(s).isEmpty()?"":s;
    }
    static String origin(String url) {
        try { URI u=new URI(url); String h=host(url); return h.isEmpty()?"":"https://"+h+(u.getPort()!=-1&&u.getPort()!=443?":"+u.getPort():""); }
        catch(Exception e){return "";}
    }
    static boolean matches(Set<String> rules, String host) {
        if(host.isEmpty())return false;
        String part=host;
        while(true){if(rules.contains(part))return true;int dot=part.indexOf('.');if(dot<0)return false;part=part.substring(dot+1);}
    }
}
