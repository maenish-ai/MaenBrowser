"""Publisher-side verification of a signed DATA artifact against the shipped trust root."""
import argparse,base64,hashlib,json,re
from pathlib import Path
from datetime import datetime,timezone
from cryptography.hazmat.primitives import serialization,hashes
from cryptography.hazmat.primitives.asymmetric import padding,rsa
ROOT=Path(__file__).resolve().parents[1]
def verify(directory,policy,sequence,now=None):
 if not policy.get('enabled') or policy.get('algorithm')!='RSA-PSS-SHA256' or not policy.get('publicKeySpki'):raise ValueError('App publisher trust root is not enabled')
 envelope=json.loads((directory/'current.json').read_text(encoding='utf-8'))
 if envelope.get('schema')!=1 or envelope.get('algorithm')!='RSA-PSS-SHA256':raise ValueError('Invalid envelope')
 raw=base64.b64decode(envelope['payload'],validate=True);sig=base64.b64decode(envelope['signature'],validate=True)
 key=serialization.load_der_public_key(base64.b64decode(policy['publicKeySpki'],validate=True))
 if not isinstance(key,rsa.RSAPublicKey) or not 2048<=key.key_size<=4096:raise ValueError('Unsupported publisher key')
 key.verify(sig,raw,padding.PSS(mgf=padding.MGF1(hashes.SHA256()),salt_length=32),hashes.SHA256())
 data=json.loads(raw)
 if data.get('product')!='MaenBrowser-security-data' or data.get('sequence')!=sequence or not 0<sequence<=9007199254740991:raise ValueError('Wrong product or sequence')
 now=now or datetime.now(timezone.utc)
 issued=datetime.fromisoformat(data['issuedUtc'].replace('Z','+00:00'));expiry=datetime.fromisoformat(data['expiresUtc'].replace('Z','+00:00'))
 if issued>now or expiry<=now or (expiry-issued).total_seconds()>31*86400:raise ValueError('Invalid feed lifetime')
 for kind in ['ads','adult','violence']:
  item=data['lists'][kind];content=(directory/(kind+'.txt')).read_bytes()
  expected=f'https://raw.githubusercontent.com/maenish-ai/MaenBrowser/security-data/{sequence}/{kind}.txt'
  if item['url']!=expected or len(content)!=item['size'] or hashlib.sha256(content).hexdigest()!=item['sha256']:raise ValueError('Data integrity or URL mismatch: '+kind)
 return True
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--directory',required=True,type=Path);p.add_argument('--sequence',required=True,type=int);a=p.parse_args()
 verify(a.directory,json.loads((ROOT/'assets/companion/security-update-policy.json').read_text()),a.sequence);print('Publisher root, signature, sequence, expiry and all list hashes verified')
