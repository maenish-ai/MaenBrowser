// Rates are sampled locally only while the downloads page is visible.
export function sampleProgress(item,previous,now){
  const bytes=Math.max(0,Number(item.bytesReceived)||0);
  const total=Number(item.totalBytes)>0?Number(item.totalBytes):null;
  const active=item.state==='in_progress'&&!item.paused;
  let rate=null;
  if(active&&previous?.active&&now>previous.time&&bytes>=previous.bytes)
    rate=(bytes-previous.bytes)*1000/(now-previous.time);
  return {bytes,total,rate,percent:total?Math.min(100,bytes/total*100):null,
    seconds:rate>0&&total?Math.max(0,(total-bytes)/rate):null,
    sample:{bytes,time:now,active}};
}
export function formatBytes(bytes){
  const n=Math.max(0,Number(bytes)||0);
  if(n<1024)return `${Math.round(n)} B`;
  if(n<1048576)return `${(n/1024).toFixed(1)} KiB`;
  if(n<1073741824)return `${(n/1048576).toFixed(1)} MiB`;
  return `${(n/1073741824).toFixed(2)} GiB`;
}
