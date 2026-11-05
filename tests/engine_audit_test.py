from pathlib import Path
import importlib.util
p=Path(__file__).resolve().parents[1]/'tools/audit-engine.py'
spec=importlib.util.spec_from_file_location('audit',p);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
lock={'cef_version':'1.0.0+gabc+chromium-154.0.8037.98'}
chrome={'versions':[{'version':'154.0.8037.98'},{'version':'156.0.8078.80'}]}
cef={'windows64':{'versions':[{'channel':'stable','cef_version':'new','chromium_version':'156.0.8078.80'},{'channel':'beta','cef_version':'beta','chromium_version':'158.0.1.1'}]}}
r=m.report(lock,chrome,cef);assert not r['currentAgainstChrome'] and not r['currentAgainstCef'];assert r['latestCefStable']=='new'
lock['cef_version']='1.0.0+gabc+chromium-156.0.8078.80';assert m.report(lock,chrome,cef)['currentAgainstChrome']
try:m.version('156.0.1.1 unexpected')
except ValueError:pass
else:raise AssertionError('Malformed version accepted')
print('Engine freshness comparison: stable channels, full version ordering and malformed sources passed')
