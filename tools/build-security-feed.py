"""Build a data-only RSA-PSS signed feed from reviewed filter files. Never uploads."""
import argparse,base64,hashlib,json,os,re,shutil
from datetime import datetime,timedelta,timezone
from pathlib import Path

def build(root,output,sequence,private_key_pem,minimum='1.13.0',now=None):
 from cryptography.hazmat.primitives import serialization,hashes
 from cryptography.hazmat.primitives.asymmetric import rsa,padding
 key=serialization.load_pem_private_key(private_key_pem,password=None)
 if not isinstance(key,rsa.RSAPrivateKey) or not 2048<=key.key_size<=4096:raise ValueError('Expected publisher RSA key of2048–4096bits')
 if not 0<sequence<=9007199254740991:raise ValueError('Invalid sequence')
 now=now or datetime.now(timezone.utc);output.mkdir(parents=True,exist_ok=True);lists={};encoded_size=0
 notices=output/'notices';notices.mkdir(exist_ok=True)
 shutil.copyfile(root/'THIRD_PARTY_NOTICES.md',notices/'THIRD_PARTY_NOTICES.md')
 for source in (root/'assets/filters/sources').glob('*LICENSE*'):shutil.copyfile(source,notices/source.name)
 for kind,minimum_count in [('ads',10000),('adult',10000),('violence',100)]:
  data=(root/'assets/filters'/f'{kind}.txt').read_bytes()
  if kind=='ads':
   merged=set(data.decode('ascii').splitlines())|set((root/'assets/filters/ads-extra.txt').read_text(encoding='ascii').splitlines())
   data=('\n'.join(sorted(merged))+'\n').encode('ascii')
  lines=data.decode('ascii').splitlines();encoded_size+=len(data)+len(lines)
  if any(len(s)>253 or '.' not in s or not all(re.fullmatch(r'[a-z0-9](?:[a-z0-9-]{0,61}[a-z0-9])?',p) for p in s.split('.')) for s in lines) or not minimum_count<=len(lines)<=250000 or lines!=sorted(set(lines)) or not data.endswith(b'\n') or len(data)>4*1024*1024:raise ValueError('Invalid reviewed list: '+kind)
  (output/f'{kind}.txt').write_bytes(data)
  lists[kind]={'url':f'https://raw.githubusercontent.com/maenish-ai/MaenBrowser/security-data/{sequence}/{kind}.txt','size':len(data),'sha256':hashlib.sha256(data).hexdigest()}
 if encoded_size+1024>5*1024*1024:raise ValueError('Reviewed data exceeds the bounded native transaction size')
 payload={'schema':1,'product':'MaenBrowser-security-data','sequence':sequence,'minimumAppVersion':minimum,'issuedUtc':now.isoformat().replace('+00:00','Z'),'expiresUtc':(now+timedelta(days=14)).isoformat().replace('+00:00','Z'),'lists':lists}
 encoded=json.dumps(payload,separators=(',',':'),sort_keys=True).encode()
 signature=key.sign(encoded,padding.PSS(mgf=padding.MGF1(hashes.SHA256()),salt_length=32),hashes.SHA256())
 envelope={'schema':1,'algorithm':'RSA-PSS-SHA256','payload':base64.b64encode(encoded).decode(),'signature':base64.b64encode(signature).decode()}
 (output/'current.json').write_text(json.dumps(envelope,indent=2)+'\n')
 public=key.public_key().public_bytes(serialization.Encoding.DER,serialization.PublicFormat.SubjectPublicKeyInfo)
 (output/'publisher-public-key.txt').write_text(base64.b64encode(public).decode()+'\n')
 return envelope
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--output',required=True,type=Path);p.add_argument('--sequence',required=True,type=int);p.add_argument('--private-key-env',default='MAEN_SECURITY_SIGNING_KEY_PEM');a=p.parse_args()
 secret=os.environ.get(a.private_key_env)
 if not secret:raise SystemExit('Publisher signing secret is missing; no feed was produced.')
 build(Path(__file__).resolve().parents[1],a.output,a.sequence,secret.encode());print('Signed data artifacts prepared locally; no publication performed.')
