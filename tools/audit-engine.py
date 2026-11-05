"""Report the locked Windows engine against official current stable versions.

No scanner claim: version comparison cannot establish absence of vulnerabilities.
Network/source failures are explicit, never treated as up-to-date.
"""
import argparse,json,re,sys
from pathlib import Path
from urllib.request import Request,urlopen
from datetime import datetime,timezone
ROOT=Path(__file__).resolve().parents[1]
def get(url):
    with urlopen(Request(url,headers={'User-Agent':'MaenBrowser-engine-audit/1'}),timeout=40) as r:
        data=r.read(32*1024*1024+1)
        if len(data)>32*1024*1024:raise ValueError('Upstream response too large')
        return json.loads(data)
def version(value):
    if not re.fullmatch(r'\d+\.\d+\.\d+\.\d+',value):raise ValueError('Unexpected engine version')
    return tuple(map(int,value.split('.')))
def report(lock,chrome,cef):
    pinned=lock['cef_version'].split('+chromium-',1)[1]
    stable=[x for x in cef['windows64']['versions'] if x.get('channel')=='stable']
    newest=max(stable,key=lambda x:version(x['chromium_version']))
    latest=max((x['version'] for x in chrome['versions']),key=version)
    return {'schema':1,'checkedUtc':datetime.now(timezone.utc).isoformat(),'pinnedChromium':pinned,'latestChromeStable':latest,'latestCefStable':newest['cef_version'],'latestCefChromium':newest['chromium_version'],'currentAgainstChrome':version(pinned)>=version(latest),'currentAgainstCef':version(pinned)>=version(newest['chromium_version']),'note':'A matching version is not proof that the binary is secure. Codec and sandbox runtime tests remain mandatory.'}
def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--output',type=Path,required=True);p.add_argument('--require-current',action='store_true');a=p.parse_args()
    try:
        result=report(json.loads((ROOT/'config/media-runtime.lock.json').read_text(encoding='utf-8')),get('https://versionhistory.googleapis.com/v1/chrome/platforms/win/channels/stable/versions?page_size=20'),get('https://cef-builds.spotifycdn.com/index.json'))
        code=2 if a.require_current and not result['currentAgainstChrome'] else 0
    except Exception as e:
        result={'schema':1,'status':'unknown','error':str(e),'currentAgainstChrome':False};code=2 if a.require_current else 1
    a.output.parent.mkdir(parents=True,exist_ok=True);a.output.write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8');print(json.dumps(result,indent=2));return code
if __name__=='__main__':sys.exit(main())
