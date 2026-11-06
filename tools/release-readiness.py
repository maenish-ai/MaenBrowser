"""Fail closed for production publication; normal source/test builds remain available."""
import argparse,json,sys
from datetime import datetime,timezone,timedelta
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def blockers(readiness,identity,engine,version,now=None):
 errors=[]
 now=now or datetime.now(timezone.utc)
 try:
  checked=datetime.fromisoformat(engine.get('checkedUtc','').replace('Z','+00:00'))
  if checked.tzinfo is None or checked>now+timedelta(minutes=5) or now-checked>timedelta(days=1):errors.append('Engine freshness evidence is stale or future dated')
 except (ValueError,TypeError):errors.append('Engine freshness timestamp is missing or invalid')
 if readiness.get('schema')!=1 or readiness.get('version')!=version:errors.append('Release review does not match VERSION')
 for flag in ['publisherReviewed','engineAdvisoryReviewComplete','physicalDeviceAcceptanceComplete','storeIdentityConfirmed']:
  if readiness.get(flag) is not True:errors.append('Acceptance missing: '+flag)
 if identity.get('configured') is not True or not identity.get('androidIdentityConfirmed'):errors.append('Publisher-owned Store identities are not confirmed')
 if engine.get('currentAgainstCef') is not True:errors.append('Engine is behind official stable CEF or freshness is unknown')
 return errors
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--engine-report',required=True,type=Path);p.add_argument('--require',action='store_true');a=p.parse_args()
 errors=blockers(json.loads((ROOT/'config/release-readiness.json').read_text(encoding='utf-8')),json.loads((ROOT/'config/store-identity.json').read_text(encoding='utf-8')),json.loads(a.engine_report.read_text(encoding='utf-8')),(ROOT/'VERSION').read_text().strip())
 pinned=json.loads((ROOT/'config/media-runtime.lock.json').read_text(encoding='utf-8'))['cef_version'].split('+chromium-',1)[1]
 if json.loads(a.engine_report.read_text(encoding='utf-8')).get('pinnedChromium')!=pinned:errors.append('Engine evidence does not match the selected production runtime')
 print(json.dumps({'releaseReady':not errors,'blockers':errors},indent=2));sys.exit(1 if errors and a.require else 0)
