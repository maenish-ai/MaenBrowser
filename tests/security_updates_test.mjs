import assert from 'node:assert/strict';
import {webcrypto} from 'node:crypto';
Object.defineProperty(globalThis,'crypto',{value:webcrypto,configurable:true});
const {verifySecurityEnvelope,fetchSecurityList}=await import('../assets/companion/security-updates.js');
const keys=await crypto.subtle.generateKey({name:'RSA-PSS',modulusLength:2048,publicExponent:new Uint8Array([1,0,1]),hash:'SHA-256'},true,['sign','verify']);
const b64=bytes=>Buffer.from(bytes).toString('base64');
const policy={schema:1,enabled:true,algorithm:'RSA-PSS-SHA256',publicKeySpki:b64(await crypto.subtle.exportKey('spki',keys.publicKey)),allowedDownloadOrigin:'https://github.com',allowedDownloadPrefix:'/maenish-ai/MaenBrowser/releases/download/security-data-'};
const data={schema:1,product:'MaenBrowser-security-data',sequence:2,minimumAppVersion:'1.13.0',issuedUtc:'2026-11-06T00:00:00Z',expiresUtc:'2026-11-20T00:00:00Z',lists:{ads:{url:'https://github.com/maenish-ai/MaenBrowser/releases/download/security-data-2/ads.txt',sha256:'a'.repeat(64),size:200000}}};
async function sign(d){const bytes=new TextEncoder().encode(JSON.stringify(d));return {schema:1,algorithm:'RSA-PSS-SHA256',payload:b64(bytes),signature:b64(await crypto.subtle.sign({name:'RSA-PSS',saltLength:32},keys.privateKey,bytes))};}
data.lists.adult={...data.lists.ads};data.lists.violence={...data.lists.ads};
const options={installed:'1.13.0',acceptedSequence:1,now:Date.parse('2026-11-06T12:00:00Z')};
const envelope=await sign(data);assert.equal((await verifySecurityEnvelope(envelope,policy,options)).sequence,2);
await assert.rejects(()=>verifySecurityEnvelope(envelope,policy,{...options,acceptedSequence:2}),/sequence/);
await assert.rejects(()=>verifySecurityEnvelope({...envelope,signature:envelope.signature.replace(/^./,envelope.signature[0]==='A'?'B':'A')},policy,options),/signature/);
await assert.rejects(()=>verifySecurityEnvelope(envelope,policy,{...options,now:Date.parse('2026-12-01')}),/expired/);
await assert.rejects(()=>verifySecurityEnvelope(envelope,policy,{...options,installed:'1.12.2'}),/newer browser/);
await assert.rejects(()=>verifySecurityEnvelope(envelope,{...policy,enabled:false},options),/not configured/);
const bad=structuredClone(data);bad.lists.ads.url='https://evil.example/list';await assert.rejects(async()=>verifySecurityEnvelope(await sign(bad),policy,options),/Untrusted/);
const bytes=new TextEncoder().encode('wrong\n');const mismatchFetcher=async()=>new Response(bytes);
await assert.rejects(()=>fetchSecurityList({...data.lists.ads,size:bytes.length},policy,mismatchFetcher),/integrity/);
console.log('Signed data: real RSA-PSS verification, tamper, replay, expiry, app floor, size/hash and origin rejection passed');
// Complete client transaction with real signatures/hash checks and controlled transport.
const {checkSecurityData}=await import('../assets/companion/security-updates.js');
const texts={ads:Array.from({length:10000},(_,i)=>`ad${String(i).padStart(5,'0')}.example\n`).join(''),adult:Array.from({length:10000},(_,i)=>`site${String(i).padStart(5,'0')}.example\n`).join(''),violence:Array.from({length:100},(_,i)=>`site${String(i).padStart(5,'0')}.example\n`).join('')};
const current={...data,sequence:10,issuedUtc:new Date(Date.now()-60000).toISOString(),expiresUtc:new Date(Date.now()+86400000).toISOString(),lists:{}};
for(const [kind,text] of Object.entries(texts)){
 const payload=new TextEncoder().encode(text);current.lists[kind]={url:`https://github.com/maenish-ai/MaenBrowser/releases/download/security-data-10/${kind}.txt`,size:payload.length,sha256:Buffer.from(await crypto.subtle.digest('SHA-256',payload)).toString('hex')};
}
const signed=await sign(current);let state={},family=false,hasPin=false,failWrite=false,nativeSequence='0',nativeCalls=0,ackLost=false;
globalThis.chrome={runtime:{getURL:f=>'https://local.example/'+f,getManifest:()=>({version:'1.13.0'})}};
const configured={...policy,feedUrl:'https://publisher.example/current.json'};
const fetcher=async url=>{
 if(url.endsWith('security-update-policy.json'))return Response.json(configured);
 if(url===configured.feedUrl)return Response.json(signed);
 return new Response(texts[url.split('/').pop().replace('.txt','')]);
};
const storage={get:async defaults=>({...defaults,...state}),set:async values=>Object.assign(state,values)};
const api=async(op='get',args)=>{
 if(op==='get')return {version:'1.13.0',revision:'3',family,hasPin,securityDataSequence:nativeSequence};
 nativeCalls++;assert.equal(op,'updateSecurityData');assert.equal(args.expectedRevision,'3');assert.equal(args.lists.adult,texts.adult);
 if(failWrite)throw Error('disk full');nativeSequence=args.sequence;if(ackLost)throw Error('response lost');return {ok:true};
};
family=true;assert.equal((await checkSecurityData({api,storage,fetcher})).status,'parent-required');assert.equal(nativeCalls,0);
family=false;hasPin=true;assert.equal((await checkSecurityData({api,storage,fetcher,automatic:true})).status,'parent-required');
failWrite=true;await assert.rejects(()=>checkSecurityData({api,storage,fetcher,pin:'test-only'}),/disk full/);assert.equal(state.securityDataAccepted,undefined);
failWrite=false;assert.equal((await checkSecurityData({api,storage,fetcher,pin:'test-only'})).status,'updated');assert.equal(state.securityDataAccepted,10);
assert.equal((await checkSecurityData({api,storage,fetcher})).status,'up-to-date');
console.log('Signed data transaction: parent lock, real three-list hashes, native atomic acknowledgement, failure preservation and current-version no-op passed');

state={};nativeSequence='0';hasPin=false;ackLost=true;
await assert.rejects(()=>checkSecurityData({api,storage,fetcher}),/response lost/);
assert.equal(state.securityDataAccepted,undefined);assert.equal(nativeSequence,'10');ackLost=false;
assert.equal((await checkSecurityData({api,storage,fetcher})).status,'up-to-date');assert.equal(state.securityDataAccepted,10);
console.log('Signed data: lost acknowledgement reconciles with protected native floor without replaying an install');
