"""Rebuild the review CSV from retained public-source facts (no network or user data)."""
from pathlib import Path
from collections import defaultdict
import csv,datetime,json,re
ROOT=Path(__file__).resolve().parents[2]
FACTS=ROOT/'docs/research/2026-11-05/community-source-facts.json'
FIELDS='id canonical_key feature_name problem source_url published_date date_precision platform votes vote_kind comments distinct_requesters_in_sample requester_count_basis request_type target_os source_environment competitors_offering competitor_evidence maen_status code_evidence complexity_estimate resource_impact_estimate security_privacy_impact feasibility priority demand_score ux_score stability_score security_score ease_score recency_factor weighted_score supporting_urls evidence_quality'.split()

def assessment(key):
    # Conservative exact-refinement defaults: these are engineering estimates,
    # not runtime certifications or inferred competitor implementations.
    result=dict(maen_status='Not verified for this exact refinement',code_evidence='docs/MAENBROWSER_CURRENT_STATE_AUDIT.md',complexity_estimate='Medium (design estimate)',resource_impact_estimate='Unmeasured; optional UI should avoid background polling',security_privacy_impact='Requires input validation and permission review',feasibility='Needs scoped prototype and Windows/Android acceptance',priority='P3',ux_score=2,stability_score=2,security_score=2,ease_score=3,competitors_offering='Not verified for exact requested refinement',competitor_evidence='docs/COMPETITOR_FEATURE_COMPARISON.md')
    if any(w in key for w in ['tab','workspace','session']):
        result.update(code_evidence='assets/companion/tabs.js; assets/companion/session-restore.js; assets/companion/tab-policy.js',priority='P2',ux_score=4,stability_score=3,ease_score=2,complexity_estimate='High: native runtime/API and persistence validation',resource_impact_estimate='May retain renderers; deferred restore can reduce active load, unmeasured')
    if any(w in key for w in ['profile','container','cookie']):
        result.update(code_evidence='src/app/maen_app.cpp; assets/companion/tabs.js',priority='P1',ux_score=4,security_score=5,ease_score=1,complexity_estimate='High: storage partition and identity isolation',security_privacy_impact='Never copy cookies/tokens across profiles or engines',resource_impact_estimate='Extra processes/storage per profile; unmeasured')
    if any(w in key for w in ['theme','toolbar','color','menu','sidebar','button','speed-dial','shortcut','hover']):
        result.update(priority='P2',ux_score=3,ease_score=3,code_evidence='assets/companion/ui.css; assets/companion/start.html; src/app/maen_app.cpp',resource_impact_estimate='Expected low if static and event-driven; unmeasured')
    if any(w in key for w in ['sync','self-host']):
        result.update(priority='P2',ux_score=4,security_score=5,ease_score=1,complexity_estimate='High: end-to-end encryption, pairing, recovery and service',security_privacy_impact='Opt-in E2EE; no public listener; secrets must not be logged',maen_status='No sync transport identified; local session export is not sync',resource_impact_estimate='Network and persistence costs; unmeasured')
    if any(w in key for w in ['scriptlet','headers','usercss','userstyles']):
        result.update(priority='P3',security_score=1,ease_score=1,security_privacy_impact='High risk of arbitrary script or credential/header exposure; no remote executable filter updates',feasibility='Restricted design review; not approved for arbitrary remote code')
    if any(w in key for w in ['memory-pressure','unload-reliability','idle-cpu','challenge-compatibility','extension-compatibility']):
        result.update(priority='P1',ux_score=5,stability_score=5,ease_score=2,code_evidence='assets/companion/memory-policy.js; assets/companion/idle-page-probe.js; src/extensions/extension_manager.cpp',resource_impact_estimate='Measure entire process tree; no fixed memory target promised')
    if any(w in key for w in ['preserve-sessions-during-upgrade','login-cookie-durability','don-t-survive-a-restart','black-screen']):
        result.update(priority='P0',ux_score=5,stability_score=5,security_score=4,ease_score=2,feasibility='Reproduce first; P0 category is a safety priority, not a confirmed MaenBrowser defect')
    overrides={
      'native-ad-blocking':('Present; native + companion policies, website-specific efficacy unverified','src/protection/; assets/companion/filter-update.js','Brave; Vivaldi; Opera (settings vary)','https://support.brave.app/hc/en-us/articles/15872271230221-What-are-all-the-features-Brave-Browser-offers'),
      'custom-search-engines':('Present; targeted portable tests, actual UI acceptance still required','assets/companion/start.js; assets/companion/options.js','Chrome; Edge; Firefox; Brave; Vivaldi; Opera','https://www.google.com/chrome/tips/'),
      'restore-browser-session':('Partial: saved URL/group snapshots and deferred restore; not full unsaved DOM state','assets/companion/session-restore.js; src/app/browser_preferences.cpp','Chrome; Edge; Firefox; Brave; Vivaldi; Opera','https://www.google.com/chrome/tips/'),
      'vertical-tabs':('Companion tab manager exists; native vertical tab strip not verified','assets/companion/tabs.js','Firefox; Brave; Vivaldi; Edge; Opera (channel/settings vary)','https://blog.mozilla.org/en/firefox/vertical-tabs-and-the-firefox-community/'),
      'split-view':('Native implementation not identified; link organizer is not split rendering','assets/companion/tabs.js','Chrome; Edge; Vivaldi; Opera','https://support.google.com/chrome/answer/16971124'),
      'workspaces':('Partial: companion saved sessions/groups; isolated native workspaces not verified','assets/companion/session-restore.js','Edge; Vivaldi; Opera','https://help.vivaldi.com/desktop/tabs/workspaces/'),
      'pinned-tabs':('Chrome Runtime tab APIs used; persistence needs native acceptance','assets/companion/tabs.js','Chrome; Edge; Firefox; Brave; Vivaldi; Opera','https://www.google.com/chrome/tips/'),
      'manual-tab-hibernation':('Partial: discard policy guards; requested context-menu refinement unverified','assets/companion/tab-policy.js; assets/companion/idle-page-probe.js','Vivaldi hibernation; others memory-saving variants','https://help.vivaldi.com/desktop/tabs/workspaces/'),
      'read-aloud':('Not identified in current app','assets/companion/reader.js','Edge; other platform availability unverified','https://www.microsoft.com/en-us/edge/features/read-aloud'),
      'optional-heavy-features':('Present resource modes; lightness on real devices not yet measured','src/app/; assets/companion/memory-policy.js','Optionality varies; no uniform equivalence asserted','docs/PERFORMANCE_BENCHMARKS.md'),
    }
    if key in overrides:
        a,b,c,d=overrides[key];result.update(maen_status=a,code_evidence=b,competitors_offering=c,competitor_evidence=d)
    return result

def build():
    groups=defaultdict(list)
    for x in json.loads(FACTS.read_text()):groups[x['canonical_key']].append(x)
    rows=[]
    for i,(key,items) in enumerate(sorted(groups.items()),1):
        # Prefer a recently CREATED post, not a recently bumped old topic.
        recent=[x for x in items if x.get('date','')[:10]>='2024-11-05']
        rep=(recent or items)[0];known={x['requester_account_hash'] for x in items if x['requester_account_hash']};platforms={x['platform'] for x in items}
        demand=5 if len(platforms)>=4 and len(known)>=10 else 4 if len(platforms)>=3 else 3 if len(platforms)>=2 and len(known)>=2 else 2 if len(known)>=2 else 1
        recency=1.0 if recent else .5
        a=assessment(key);title=' '.join(rep['title'].split()[:24])
        kind='bug fix' if any(w in key for w in ['bug','misplacement','don-t-survive','reliability','durability','preserve-sessions','black-screen','unable-to','not-working','logging-out','verification-loop']) else 'feature request'
        systems={x.get('system','Both') for x in items};system='Both' if 'Both' in systems or len(systems)>1 else next(iter(systems))
        row=dict(id=f'CR-{i:03}',canonical_key=key,feature_name=title,problem=f'Sampled users seek {key.replace("-"," ")}; this records their workflow request, not proof of a MaenBrowser defect.',source_url=rep['url'],published_date=rep.get('date',''),date_precision='day' if len(rep.get('date',''))==10 else 'timestamp' if rep.get('date') else 'unknown',platform=rep['platform'],votes='' if rep.get('votes') is None else rep['votes'],vote_kind='GitHub +1 reactions' if rep['platform']=='GitHub Issues' else 'NodeBB upvotes' if rep['platform']=='Vivaldi Forum' else 'Discourse vote_count' if rep['platform']=='Brave Community' else 'Mozilla upvoteCount' if rep['platform']=='Mozilla Connect' else 'unavailable',comments='' if rep.get('comments') is None else rep['comments'],distinct_requesters_in_sample=len(known),requester_count_basis='Distinct namespaced public OP/explicit-requester accounts; lower bound, not unique physical people',request_type=kind,target_os=system,source_environment=rep.get('source_environment','Not recorded'),demand_score=demand,recency_factor=recency,supporting_urls=' | '.join(sorted({x['url'] for x in items})),evidence_quality=' | '.join(sorted({x['evidence_quality'] for x in items})),**a)
        row['weighted_score']=round(100*(.25*demand*recency+.20*a['ux_score']+.20*a['stability_score']+.20*a['security_score']+.15*a['ease_score'])/5,1)
        rows.append(row)
    destination=ROOT/'docs/COMMUNITY_REQUESTS.csv'
    with destination.open('w',encoding='utf-8',newline='') as f:
        w=csv.DictWriter(f,fieldnames=FIELDS,lineterminator="\n");w.writeheader();w.writerows(rows)
    (ROOT/'COMMUNITY_REQUESTS.csv').write_bytes(destination.read_bytes())
    return rows
if __name__=='__main__':print(f'Wrote {len(build())} distinct requests')
