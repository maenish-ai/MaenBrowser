package ai.maenish.maenbrowser;
import org.junit.Test;
import static org.junit.Assert.*;
import java.util.Set;
public class WebPolicyTest {
 @Test public void upgradesHttps(){assertEquals("https://example.org/a",WebPolicy.address("http://example.org/a"));assertEquals("https://example.org",WebPolicy.address("example.org"));}
 @Test public void rejectsUnsafeNavigation(){for(String s:new String[]{"javascript:alert(1)","file:///etc/passwd","data:text/html,hi","intent://a","https://user:pass@example.org","https://","https://a\\@example.org"})assertEquals(s,"",WebPolicy.address(s));}
 @Test public void exactDomainBoundaries(){Set<String>s=Set.of("blocked.test");assertTrue(WebPolicy.matches(s,"a.blocked.test"));assertFalse(WebPolicy.matches(s,"blocked.test.evil"));assertFalse(WebPolicy.matches(s,"notblocked.test"));}
 @Test public void normalizedOrigin(){assertEquals("https://example.org",WebPolicy.origin("https://EXAMPLE.org:443/a"));assertEquals("https://example.org:8443",WebPolicy.origin("https://example.org:8443/a"));assertEquals("",WebPolicy.origin("http://example.org"));}
}
