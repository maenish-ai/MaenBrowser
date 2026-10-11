// Serializable, read-only snapshot. No listeners, timers, URLs or page changes.
export function inspectPageMedia(){
  const elements=Array.from(document.querySelectorAll('video,audio'));
  const probe=document.createElement('video');
  return {schema:1,total:elements.length,truncated:elements.length>20,
    capabilities:{h264:probe.canPlayType('video/mp4; codecs="avc1.42E01E"'),aac:probe.canPlayType('audio/mp4; codecs="mp4a.40.2"')},
    items:elements.slice(0,20).map(e=>({kind:e.tagName==='VIDEO'?'video':'audio',
      errorCode:[1,2,3,4].includes(e.error?.code)?e.error.code:null,
      readyState:e.readyState,networkState:e.networkState,paused:e.paused,ended:e.ended,
      hasSource:!!(e.currentSrc||e.srcObject),encrypted:!!e.mediaKeys,
      width:Number(e.videoWidth)||0,height:Number(e.videoHeight)||0}))};
}
