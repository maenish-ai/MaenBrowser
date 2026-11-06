"""Summarize actual Windows captures; refuse to invent missing hardware measurements."""
import argparse,json,statistics
from pathlib import Path

def summarize(data):
 samples=data.get('samples',[])
 if not samples:raise ValueError('No real samples')
 def stats(field):
  values=[float(x[field]) for x in samples if isinstance(x.get(field),(int,float))]
  if not values:return None
  return {'min':min(values),'median':statistics.median(values),'max':max(values),'samples':len(values)}
 return {'schema':1,'os':data.get('os'),'osBuild':data.get('osBuild'),'physicalMemoryMiB':data.get('physicalMemoryMiB'),'cpu':data.get('cpu'),'browserVersion':data.get('browserVersion'),'declaredTabs':data.get('declaredTabs'),'tabCountVerified':data.get('tabCountVerified',False),'scenario':data.get('scenario'),'privateCommitMiB':stats('privateCommitMiB'),'summedWorkingSetMiB':stats('summedWorkingSetMiB'),'cpuPercentAllLogicalProcessors':stats('cpuPercentAllLogicalProcessors'),'limitations':data.get('limitations',[])}
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('captures',nargs='+',type=Path);p.add_argument('--output',required=True,type=Path);a=p.parse_args()
 results=[summarize(json.loads(f.read_text(encoding='utf-8-sig'))) for f in a.captures];a.output.parent.mkdir(parents=True,exist_ok=True);a.output.write_text(json.dumps(results,indent=2)+'\n');print('Summarized',len(results),'provided captures; absent device classes remain unmeasured')
