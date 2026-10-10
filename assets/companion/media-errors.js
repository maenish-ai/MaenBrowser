export function mediaError(code){
  return ({1:['MB-201','Playback was aborted.'],2:['MB-202','Media loading failed. Check the connection or file access.'],3:['MB-203','Media decoding failed. The file may be damaged or unsupported.'],4:['MB-204','Media source or format is not supported.']})[code] || ['MB-299','Media playback failed; the engine did not report a specific cause.'];
}
