const source='https://raw.githubusercontent.com/StevenBlack/hosts/master/hosts';
export function parseHosts(text) {
  const domains=new Set();
  for(const line of text.split(/\r?\n/)) {
    const fields=line.split('#')[0].trim().split(/\s+/);
    if(fields[0]!=='0.0.0.0'&&fields[0]!=='127.0.0.1')continue;
    for(const name of fields.slice(1)) {
      const host=name.toLowerCase();
      if(host.length<=253&&host.includes('.')&&host.split('.').every(p=>/^[a-z0-9](?:[a-z0-9-]{0,61}[a-z0-9])?$/.test(p))&&host!=='localhost.localdomain')domains.add(host);
    }
  }
  if(domains.size<10000||domains.size>250000)throw new Error('Invalid ad filter snapshot. Existing filters were kept.');
  const result=[...domains].sort().join('\n')+'\n';
  if(result.length>4*1024*1024)throw new Error('Invalid ad filter snapshot. Existing filters were kept.');
  return result;
}
export async function downloadAdFilters() {
  const response=await fetch(source,{credentials:'omit',cache:'no-store',redirect:'error',signal:AbortSignal.timeout(30000)});
  if(!response.ok||!response.body)throw new Error('Filter download failed. Existing filters were kept.');
  const reader=response.body.getReader(),decoder=new TextDecoder('utf-8',{fatal:true});
  let size=0,text='';
  try { for(;;) {
    const {done,value}=await reader.read();if(done)break;
    size+=value.byteLength;
    if(size>8*1024*1024)throw new Error('Filter download too large. Existing filters were kept.');
    text+=decoder.decode(value,{stream:true});
  } text+=decoder.decode(); } finally { await reader.cancel().catch(()=>{});reader.releaseLock(); }
  return parseHosts(text);
}
