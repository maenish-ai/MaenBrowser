"""Offline configuration boundaries; real Java tests/build are separate Gradle tasks."""
from pathlib import Path
import xml.etree.ElementTree as ET
r=Path(__file__).resolve().parents[1]
b=(r/'android/app/build.gradle').read_text(encoding='utf-8')
assert 'minSdk 29' in b and 'targetSdk 36' in b
assert "applicationIdSuffix '.preview'" in b
assert 'MAEN_ANDROID_KEYSTORE' in b and "debug.keystore" not in b
assert 'testInstrumentationRunner' in b
manifest=ET.parse(r/'android/app/src/main/AndroidManifest.xml').getroot();ns='{http://schemas.android.com/apk/res/android}'
app=manifest.find('application');assert app.get(ns+'allowBackup')=='false';assert app.get(ns+'usesCleartextTraffic')=='false'
assert not manifest.findall('.//service')
java='\n'.join(p.read_text(encoding='utf-8') for p in (r/'android/app/src/main/java').rglob('*.java'))
assert 'addJavascriptInterface(' not in java and 'handler.proceed(' not in java
assert 'setWebContentsDebuggingEnabled(BuildConfig.DEBUG)' in java
assert 'MIXED_CONTENT_NEVER_ALLOW' in java and 'setAllowFileAccess(false)' in java
props=(r/'android/gradle/wrapper/gradle-wrapper.properties').read_text(encoding='utf-8')
assert 'distributionSha256Sum=20f1b1176237254a6fc204d8434196fa11a4cfb387567519c61556e8710aed78' in props
assert (r/'android/gradle/wrapper/gradle-wrapper.jar').is_file()
print('Android package: API levels, isolated preview ID, publisher signing, TLS/bridge boundaries and wrapper checksum passed (not device validation)')
