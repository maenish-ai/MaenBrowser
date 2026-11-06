# Community feature research — captured 2026-11-05

**143 distinct workflow requests, 155 unique original threads, 165 request/evidence associations.** [Request CSV](COMMUNITY_REQUESTS.csv), [extracted public-source facts](research/2026-11-05/community-source-facts.json), [source index/access limitations](COMMUNITY_SOURCE_INDEX.md), [pain points](USER_PAIN_POINTS.md), [gap matrix](FEATURE_GAP_MATRIX.md).

## Collection and evidence

Purposive discovery sample, not a representative worldwide opinion poll. Public Brave Desktop/Android feature categories, Vivaldi Desktop/Mobile requests, Mozilla Connect idea pages, GitHub browser issues, primary Reddit pages and an HN wishlist were inspected. API listing metadata selected topics; 96 Brave/Vivaldi first posts were additionally fetched, Mozilla individual HTML and GitHub issue bodies inspected, selected Reddit original posts/comments read using the page reader. The source index specifies which endpoints failed; unavailable sources are not counted as reviewed threads. All retained requests have an original source URL; no remembered generic wishlist was substituted for community evidence.

Creation date drives recency, not last comment/bump time. Recent means2024-11-05 through2026-11-05, with older or unknown publication dates downweighted. English-language public forums and highly active vendors dominate accessibility; anonymous, regional-language, private support and non-posting users are underrepresented. Community names and vendor affiliation do not establish the requester's platform usage. The CSV target_os is a proposed MaenBrowser engineering scope, not proof the reporting machine ran that OS. Source_environment retains known original systems: some Zen reports were on Linux or macOS and are transferable pain-point evidence, not reproduced Windows defects. The maintainer-only memory diagnostic tracker contributes context but no independent requester account. Threads may describe already shipped features or requests specific to another browser; these are not automatically gaps in MaenBrowser.

A canonical request represents a distinct behavior or workflow refinement, not simply a post. Examples: native split rendering, split-toolbar access and an option to disable split view are different user-control requests; duplicate manual-tab-unload posts are merged. AMOLED theme duplicates, password-generation duplicates, pinned shortcut reordering duplicates and several cross-platform variants were merged. Generic AI branding, mascot interactions, ambiguous window requests and add-on website-only layout suggestions were excluded. A single source may contain several clearly separate requests; that does not create independent supporters for each. The retained facts preserve all supporting URLs and account hashes without shipping full copyrighted posts.

Representative-row counts (not posts, votes or market share):

- Brave Community: 45
- GitHub Issues: 25
- Hacker News: 4
- Mozilla Connect: 23
- Reddit r/brave_browser: 1
- Reddit r/browsers: 7
- Reddit r/firefox: 1
- Vivaldi Forum: 37

No community accounts for a majority. The GitHub stratum includes Brave, Zen and ungoogled-chromium rather than one project's issues alone. Reddit JSON access failed403; selected primary pages were separately readable. Mozilla REST returned application errors in HTTP200; individual HTML pages worked. Edge sign-in and Chromium Issues shell did not produce a comparable structured request sample. See limitations rather than assuming those communities contributed zero demand.

## Metadata and counting

Votes/comments are the representative thread's captured public values. Empty means unavailable, **not zero**. Discourse vote_count, NodeBB upvotes, Mozilla upvoteCount and GitHub +1 reactions are labeled separately and never added into a universal popularity number. Comments are replies where the source exposes that count; they are not supporters. Distinct requesters means known distinct namespaced OP or explicitly requesting comment accounts in sampled evidence; it is a lower-bound account proxy, not verified unique humans across sites. No author is counted solely because they replied. Counts can change after capture.

Date precision is timestamp/day/unknown. Missing dates stay missing. Representative title excerpts are short identifiers; problem descriptions are paraphrases, not copied full posts. Dataset facts retain extraction hashes where captured. SHA256 identifies the fetched response; raw full HTML/JSON was kept outside the distribution and is not required for deterministic CSV regeneration.

## Priority rubric

Each component0–5, with100-point score:

`100 * (0.25*demand*recency + 0.20*UX + 0.20*stability + 0.20*security + 0.15*ease) / 5`.

Demand:1 one known requester/single community;2 at least2 accounts in one community;3 multiple accounts in at least2 communities;4 at least3 communities;5 at least4 communities and10 observed requesting accounts. Zero would mean no community evidence; engineering-only safety items are tracked separately. Recency1 for at least one recent-created supporting thread,0.5 for older/unknown only. The modest scores of single-source ideas are intentional: one popular-looking topic is not proof of global demand.

UX0 no benefit through5 essential workflow; stability0 harmful through5 prevents data loss/crashes; security0 harmful through5 closes a significant trust boundary; ease0 infeasible/unknown through5 small maintainable supported change. Scores are engineering judgments, **not measurements**. Default unprototyped UI refinements receive UX2/stability2/security2/ease3; tab structures UX4/stability3/ease2; profile isolation security5/ease1; sync security5/ease1. Overrides and per-row estimates are explicit in the reproducible [generator](../tools/research/build-community-dataset.py). Exact competitor refinements not checked remain unverified, rather than inferred from a broadly similar feature.

P0 security vulnerabilities, crashes, data loss, broken downloads and security updates override weighted demand. P1 covers interoperability, profiles, extension and resource reliability; P2 productivity/customization; P3 expensive or niche refinements and unscoped ideas. A high weighted score does not authorize an unsafe implementation, and a P0 label on another browser's bug does not assert that MaenBrowser has that defect.

## Selection and implementation

The immediate engineering lane is outdated-engine assessment, authenticated-update guard, download-context preservation, conservative tab discard and Android Family state consistency. The latter three have source/logic or build improvements in this batch; engine replacement and signed transactional update infrastructure are blocked pending real build/signing/acceptance resources. No low-priority feature was installed merely to increase the feature count.

Promising next validated prototypes: permission/capability reporting, reliable session backups, native extension acceptance fixtures, accessible command navigation and optional workspace/tab-management refinements. Native containers, split rendering and E2EE sync require separate architecture and profile-security work. Optional heavy widgets or AI are not prerequisites for browsing. Quantitative priority is advisory; [release blockers](RELEASE_BLOCKERS.md) remain authoritative.

## Reproduction / update

Run `python tools/research/build-community-dataset.py`, then `python tests/community_dataset_test.py`. This is deterministic from retained extracted facts and makes no network request. To extend the sample, fetch the documented public endpoints, inspect original requests, capture actual date/count fields, assign a canonical key, namespace/hash requesting accounts, record endpoint/result/response hash and append facts. Review semantic deduplication and platform balance before regenerating. The test checks count/arithmetic/provenance, not that every forum opinion is correct.
