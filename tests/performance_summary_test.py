from pathlib import Path
import importlib.util
p=Path(__file__).resolve().parents[1]/'tools/summarize-performance.py';s=importlib.util.spec_from_file_location('perf',p);m=importlib.util.module_from_spec(s);s.loader.exec_module(m)
x=m.summarize({'samples':[{'privateCommitMiB':10,'cpuPercentAllLogicalProcessors':None},{'privateCommitMiB':20,'cpuPercentAllLogicalProcessors':4}]})
assert x['privateCommitMiB']['median']==15 and x['cpuPercentAllLogicalProcessors']['samples']==1
try:m.summarize({'samples':[]})
except ValueError:pass
else:raise AssertionError('Empty capture accepted')
print('Performance summary: real input required, correct range/median and missing CPU interval preserved')
