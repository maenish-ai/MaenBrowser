from pathlib import Path
import importlib.util,tempfile,json,base64
from datetime import datetime,timezone
from cryptography.hazmat.primitives import serialization,hashes
from cryptography.hazmat.primitives.asymmetric import rsa,padding
root=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('feed',root/'tools/build-security-feed.py');module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
# Ephemeral test key, never a publisher trust root.
key=rsa.generate_private_key(public_exponent=65537,key_size=2048)
pem=key.private_bytes(serialization.Encoding.PEM,serialization.PrivateFormat.PKCS8,serialization.NoEncryption())
with tempfile.TemporaryDirectory() as temp:
 out=Path(temp);envelope=module.build(root,out,2026110601,pem,now=datetime(2026,11,6,tzinfo=timezone.utc))
 raw=base64.b64decode(envelope['payload']);signature=base64.b64decode(envelope['signature'])
 key.public_key().verify(signature,raw,padding.PSS(mgf=padding.MGF1(hashes.SHA256()),salt_length=32),hashes.SHA256())
 data=json.loads(raw);assert set(data['lists'])=={'ads','adult','violence'}
 assert data['sequence']==2026110601
 assert all((out/(kind+'.txt')).stat().st_size==item['size'] for kind,item in data['lists'].items())
 assert not list(out.glob('*private*'))
 assert (out/'notices/THIRD_PARTY_NOTICES.md').is_file()
 verifier_spec=importlib.util.spec_from_file_location('verify_feed',root/'tools/verify-security-feed.py');verifier=importlib.util.module_from_spec(verifier_spec);verifier_spec.loader.exec_module(verifier)
 policy={'enabled':True,'algorithm':'RSA-PSS-SHA256','publicKeySpki':(out/'publisher-public-key.txt').read_text().strip()}
 assert verifier.verify(out,policy,2026110601,now=datetime(2026,11,6,tzinfo=timezone.utc))
 (out/'ads.txt').write_bytes(b'tampered')
 try:verifier.verify(out,policy,2026110601,now=datetime(2026,11,6,tzinfo=timezone.utc))
 except ValueError:pass
 else:raise AssertionError('Publisher verifier accepted changed filter bytes')
print('Feed builder: reviewed files, real RSA-PSS signature and public-key-only output passed')
