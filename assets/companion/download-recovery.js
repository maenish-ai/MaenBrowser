export function downloadAdvice(error){
 if(['NETWORK_FAILED','NETWORK_TIMEOUT','NETWORK_DISCONNECTED','SERVER_UNREACHABLE'].includes(error))return 'Check the connection. Resume is available only when the original download supports it.';
 if(['SERVER_UNAUTHORIZED','SERVER_FORBIDDEN'].includes(error))return 'Sign in on the original website, then request the file there. This browser will not replay your login or POST request.';
 if(['FILE_NO_SPACE','FILE_ACCESS_DENIED','FILE_NAME_TOO_LONG','FILE_TOO_LARGE'].includes(error))return 'Check free space, folder permissions and the destination filename.';
 if(['FILE_VIRUS_INFECTED','FILE_BLOCKED','FILE_SECURITY_CHECK_FAILED'].includes(error))return 'The security check blocked this file. Do not disable protection to force the download.';
 return 'Return to the original website if Resume is unavailable. Your original request will not be recreated automatically.';
}
export async function resumeOriginalDownload(id,downloads=chrome.downloads){
 const items=await downloads.search({id});const item=items[0];
 if(!item||!((item.state==='in_progress'&&item.paused)||(item.state==='interrupted'&&item.canResume)))throw Error('This download cannot be resumed. Request it again on the original website.');
 // Never substitute downloads.download(url): that would lose POST/referrer/auth context.
 return downloads.resume(id);
}
