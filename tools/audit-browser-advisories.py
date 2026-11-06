"""Capture browser-related CISA KEV entries. Inclusion is not MaenBrowser exploit proof."""
import argparse,json,subprocess,re
from datetime import datetime,timezone
from pathlib import Path
SOURCE='https://raw.githubusercontent.com/cisagov/kev-data/main/known_exploited_vulnerabilities.json'
def browser_entries(data):
 result=[]
 for entry in data['vulnerabilities']:
  product=entry.get('product','')
  if not (re.search(r'(?i)chromium|chrome(?!os)|firefox|internet explorer|webkit|safari',product) or (entry.get('vendorProject')=='Microsoft' and re.search(r'(?i)\bedge\b',product))):continue
  result.append({key:entry.get(key) for key in ['cveID','vendorProject','product','vulnerabilityName','dateAdded','requiredAction','knownRansomwareCampaignUse','notes']})
 return sorted(result,key=lambda v:v.get('dateAdded',''),reverse=True)
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--output',required=True,type=Path);a=p.parse_args()
 try:
  source=subprocess.check_output(['curl','--fail','--silent','--show-error','--connect-timeout','15','--max-time','60','--max-filesize','5242880',SOURCE]);data=json.loads(source)
  result={'capturedUtc':datetime.now(timezone.utc).isoformat(),'source':SOURCE,'catalogVersion':data.get('catalogVersion'),'entries':browser_entries(data),'scope':'Historical browser-related KEV records; includes other products/platforms. Each requires exact-version, backport and reachable-feature review. No claim all entries affect MaenBrowser.'}
 except Exception as e:raise SystemExit('Advisory source unavailable: '+str(e))
 a.output.parent.mkdir(parents=True,exist_ok=True);a.output.write_text(json.dumps(result,indent=2)+'\n');print('Captured',len(result['entries']),'historical browser-related records; no automatic vulnerability verdict')
