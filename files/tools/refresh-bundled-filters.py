"""Refresh only SHA-256 pinned, bounded filter data; never run at browser startup."""
import argparse
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import re
from urllib.request import urlopen

ROOT=Path(__file__).resolve().parents[1]

def parse_domains(text,kind):
    domains=set()
    for line in text.splitlines():
        fields=line.split('#',1)[0].strip().lower().split()
        if not fields:continue
        if kind=='hosts':
            if fields[0] not in {'0.0.0.0','127.0.0.1'}:continue
            fields=fields[1:]
        for host in fields:
            if host=='localhost.localdomain' or len(host)>253 or '.' not in host:continue
            if all(re.fullmatch(r'[a-z0-9](?:[a-z0-9-]{0,61}[a-z0-9])?',p) for p in host.split('.')):domains.add(host)
    if not 10000<=len(domains)<=250000:raise ValueError('Unexpected filter domain count')
    result=('\n'.join(sorted(domains))+'\n').encode()
    if len(result)>4*1024*1024:raise ValueError('Filter too large')
    return result

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--input-dir',type=Path);args=p.parse_args()
    lock=json.loads((ROOT/'config/filter-sources.lock.json').read_text(encoding='utf-8'))
    manifest_path=ROOT/'assets/filters/manifest.json';manifest=json.loads(manifest_path.read_text(encoding='utf-8'))
    staged=[]
    for entry in lock['sources']:
        if args.input_dir:raw=(args.input_dir/('maen-filter-'+entry['name']+'.txt')).read_bytes()
        else:
            if not entry['url'].startswith('https://raw.githubusercontent.com/'):raise ValueError('Unapproved filter source')
            with urlopen(entry['url'],timeout=60) as src:raw=src.read(16*1024*1024+1)
        if len(raw)>16*1024*1024 or hashlib.sha256(raw).hexdigest()!=entry['source_sha256']:raise ValueError('Filter source hash mismatch')
        data=parse_domains(raw.decode('utf-8'),entry['format']);staged.append((entry,data))
    supplement=lock['ads_extra_supplement']
    if args.input_dir: raw=(args.input_dir/'maen-filter-ads-supplement.txt').read_bytes()
    else:
        if not supplement['url'].startswith('https://raw.githubusercontent.com/'):raise ValueError('Unapproved supplement source')
        with urlopen(supplement['url'],timeout=60) as src:raw=src.read(16*1024*1024+1)
    if len(raw)>16*1024*1024 or hashlib.sha256(raw).hexdigest()!=supplement['source_sha256']:raise ValueError('Supplement source hash mismatch')
    supplement_domains=parse_domains(raw.decode('utf-8'),'domains').decode('ascii').splitlines()
    for index,(entry,data) in enumerate(staged):
        if entry['name']=='ads-extra':
            selected={host for host in supplement_domains if host.startswith(tuple(supplement['prefixes']))}
            merged=set(data.decode('ascii').splitlines())|selected
            data=('\n'.join(sorted(merged))+'\n').encode('ascii')
            if len(data)>4*1024*1024 or len(merged)>250000:raise ValueError('Combined filter too large')
            staged[index]=(entry,data)
    for entry,data in staged:
        (ROOT/'assets/filters'/(entry['name']+'.txt')).write_bytes(data)
        item={**{k:entry[k] for k in ['name','url','source_sha256','license','commit']},'retrieved':datetime.now(timezone.utc).date().isoformat(),'sha256':hashlib.sha256(data).hexdigest(),'domains':len(data.splitlines())}
        if entry['name']=='ads-extra':item['supplement']=supplement
        manifest=[m for m in manifest if m['name']!=entry['name']]+[item]
        print(entry['name'],item['domains'])
    manifest_path.write_text(json.dumps(manifest,indent=2)+'\n',encoding='utf-8')

if __name__=='__main__':main()
