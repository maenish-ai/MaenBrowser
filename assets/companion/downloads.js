import {$,message} from './api.js';
function action(label,fn){const b=document.createElement('button');b.className='small';b.textContent=label;b.onclick=()=>Promise.resolve(fn()).catch(e=>message($('status'),e));return b;}
async function render(){
  const items=await chrome.downloads.search({orderBy:['-startTime'],limit:100});$('items').replaceChildren();
  for(const d of items){const row=document.createElement('div');row.className='tab';const info=document.createElement('div');const name=document.createElement('div');name.className='tab-title';name.textContent=d.filename.split(/[\\/]/).pop();const detail=document.createElement('small');detail.textContent=`${d.state}${d.paused?' · Paused':''} · ${(d.bytesReceived/1048576).toFixed(1)} MB${d.totalBytes>0?' / '+(d.totalBytes/1048576).toFixed(1)+' MB':''}`;info.append(name,detail);const actions=document.createElement('div');actions.className='actions';
    if(d.state==='in_progress'){actions.append(action(d.paused?'Resume':'Pause',()=>d.paused?chrome.downloads.resume(d.id):chrome.downloads.pause(d.id)),action('Cancel',()=>chrome.downloads.cancel(d.id)));}
    if(d.state==='interrupted'&&d.canResume)actions.append(action('Resume',()=>chrome.downloads.resume(d.id)));
    if(d.state==='complete'&&d.exists)actions.append(action('Open',()=>chrome.downloads.open(d.id)),action('Show folder',()=>chrome.downloads.show(d.id)));
    row.append(info,actions);$('items').append(row);
  }
  if(!items.length)$('items').textContent='No downloads yet.';
}
$('folder').onclick=()=>chrome.downloads.showDefaultFolder();$('refresh').onclick=()=>render().catch(e=>message($('status'),e));
chrome.downloads.onChanged.addListener(()=>render());chrome.downloads.onCreated.addListener(()=>render());render().catch(e=>message($('status'),e));
