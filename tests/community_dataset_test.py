"""Check research arithmetic/provenance, not the truth of a product claim."""
from pathlib import Path
import csv,datetime,hashlib,json,runpy
root=Path(__file__).resolve().parents[1]
p=root/'docs/COMMUNITY_REQUESTS.csv'; before=p.read_bytes()
runpy.run_path(str(root/'tools/research/build-community-dataset.py'),run_name='__main__')
# CSV is reproducible from the retained extraction facts.
assert before==p.read_bytes(), 'Regenerate community CSV before committing'
assert (root/'COMMUNITY_REQUESTS.csv').read_bytes()==p.read_bytes()
rows=list(csv.DictReader(p.open(encoding='utf-8', newline='')));assert len(rows)>=100
assert len({r['canonical_key'] for r in rows})==len(rows)
facts=json.loads((root/'docs/research/2026-11-05/community-source-facts.json').read_text(encoding='utf-8'))
for r in rows:
 matching=[f for f in facts if f['canonical_key']==r['canonical_key']]
 assert r['source_url'].startswith('https://') and r['source_url'] in {f['url'] for f in matching}
 assert int(r['distinct_requesters_in_sample'])==len({f['requester_account_hash'] for f in matching if f['requester_account_hash']})
 for field in ['votes','comments']:
  assert r[field]=='' or int(r[field])>=0
 if r['published_date']:assert r['published_date'][:10]<='2026-11-05'
 score=100*(.25*float(r['demand_score'])*float(r['recency_factor'])+.20*float(r['ux_score'])+.20*float(r['stability_score'])+.20*float(r['security_score'])+.15*float(r['ease_score']))/5
 assert round(score,1)==float(r['weighted_score'])
 assert r['priority'] in ['P0','P1','P2','P3']
 for field in ['feature_name','problem']:assert not r[field].startswith(('=','+','-','@'))
print(f'Community dataset: {len(rows)} distinct keys, traceable sources, counts and reproducible weighted scores')
