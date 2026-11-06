from pathlib import Path
import importlib.util
p=Path(__file__).resolve().parents[1]/'tools/store-readiness.py';s=importlib.util.spec_from_file_location('store',p);m=importlib.util.module_from_spec(s);s.loader.exec_module(m)
i={'androidIdentityConfirmed':True,'androidApplicationId':'ai.maenish.maenbrowser','privacyPolicyUrl':'https://publisher.example/privacy','supportUrl':'https://publisher.example/support'}
assert not m.report(i,'android')['missing'];assert m.report(i,'windows')['missing']
assert m.report({**i,'androidIdentityConfirmed':False},'android')['missing']
assert m.report({**i,'privacyPolicyUrl':'http://insecure.example'},'android')['missing']
assert not m.report(i,'android')['submissionReady']
print('Store metadata: Android/Windows independence, identity/privacy prerequisites and no false submission-ready claim')
