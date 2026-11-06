import importlib.util
from pathlib import Path
from datetime import datetime,timezone
p=Path(__file__).resolve().parents[1]/'tools/release-readiness.py';s=importlib.util.spec_from_file_location('readiness',p);m=importlib.util.module_from_spec(s);s.loader.exec_module(m)
r=dict(schema=1,version='1.13.0',publisherReviewed=True,engineAdvisoryReviewComplete=True,physicalDeviceAcceptanceComplete=True,storeIdentityConfirmed=True)
i=dict(configured=True,androidIdentityConfirmed=True)
assert not m.blockers(r,i,{'currentAgainstCef':True,'checkedUtc':datetime.now(timezone.utc).isoformat()},'1.13.0')
assert m.blockers(r,i,{'currentAgainstCef':False},'1.13.0')
for k in ['publisherReviewed','engineAdvisoryReviewComplete','physicalDeviceAcceptanceComplete','storeIdentityConfirmed']:
 assert m.blockers({**r,k:False},i,{'currentAgainstCef':True,'checkedUtc':datetime.now(timezone.utc).isoformat()},'1.13.0')
assert m.blockers(r,i,{},'1.14.0')
print('Production gate: unknown/aging engine, incomplete identities, review, device acceptance and wrong release fail closed')

assert m.blockers(r,i,{'currentAgainstCef':True,'checkedUtc':'2000-01-01T00:00:00Z'},'1.13.0')
