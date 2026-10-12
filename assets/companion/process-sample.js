// CPU is normalized to total logical processor capacity, main process only.
export function processSample(previous,current){
  if(!previous||!current||!Number.isInteger(current.processId)||current.processId<=0||previous.processId!==current.processId)return null;
  const a=previous.browserProcessCpuSeconds,b=current.browserProcessCpuSeconds;
  const start=previous.sampleUptimeSeconds,end=current.sampleUptimeSeconds;
  const cores=current.logicalProcessors;
  if(![a,b,start,end,cores].every(Number.isFinite)||cores<1||
     cores!==previous.logicalProcessors||end-start<1||b<a)return null;
  const percent=(b-a)/(end-start)/cores*100;
  if(percent>100.5)return null; // inconsistent sample; don't invent utilization
  return {seconds:end-start,cpuPercent:Math.min(100,percent),scope:'main-process'};
}
