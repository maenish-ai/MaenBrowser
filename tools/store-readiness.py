"""Report genuine Store prerequisites. Does not fabricate publisher ownership or signatures."""
import argparse,json,re
from pathlib import Path
from urllib.parse import urlsplit
ROOT=Path(__file__).resolve().parents[1]
def report(identity,platform='all'):
 missing=[]
 if platform in ['all','windows']:
  if identity.get('configured') is not True:missing.append('configured')
  for k in ['windowsIdentityName','windowsPublisher','windowsPublisherDisplayName']:
   if not isinstance(identity.get(k),str) or not identity[k].strip():missing.append(k)
 if platform in ['all','android'] and identity.get('androidIdentityConfirmed') is not True:missing.append('androidIdentityConfirmed')
 for k in ['privacyPolicyUrl','supportUrl']:
  u=urlsplit(identity.get(k,''))
  if u.scheme!='https' or not u.hostname or u.username or u.password:missing.append(k)
 if platform in ['all','android'] and not re.fullmatch(r'[A-Za-z][\w]*(?:\.[A-Za-z][\w]*)+',identity.get('androidApplicationId','')):missing.append('androidApplicationId')
 return {'metadataReady':not missing,'missing':missing,'additionalGates':['Publisher signing ownership and package verification','Physical Windows/Android acceptance','Store screenshots, content/permission declarations and policy review'],'submissionReady':False}
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--require-metadata',action='store_true');p.add_argument('--platform',choices=['all','windows','android'],default='all');a=p.parse_args();r=report(json.loads((ROOT/'config/store-identity.json').read_text(encoding='utf-8')),a.platform);print(json.dumps(r,indent=2));raise SystemExit(1 if r['missing'] and a.require_metadata else 0)
