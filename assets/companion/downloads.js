import {scheduleRender} from './render-scheduler.js';
import {t} from './i18n.js';
import {$,message} from './api.js';
import {sampleProgress,formatBytes} from './download-progress.js';
const rows=new Map(),samples=new Map();
let poll;
function action(label,fn){const b=document.createElement('button');b.className='small';b.textContent=t(label);b.onclick=()=>Promise.resolve(fn()).then(schedule).catch(e=>message($('status'),e));return b;}
function setText(el,text){if(el.textContent!==text)el.textContent=text;}
async function render(){
  clearTimeout(poll);
  if(document.hidden)return;
  const items=await chrome.downloads.search({orderBy:['-startTime'],limit:100});
  if(document.hidden)return;
  const now=performance.now(),ids=new Set(items.map(d=>d.id));
  for(const [id,row] of rows)if(!ids.has(id)){row.root.remove();rows.delete(id);samples.delete(id);}
  for(const d of items){
    let row=rows.get(d.id);
    if(!row){
      const root=document.createElement('div');root.className='tab';
      const info=document.createElement('div'),name=document.createElement('div'),detail=document.createElement('small'),bar=document.createElement('progress'),actions=document.createElement('div');
      name.className='tab-title';actions.className='actions';bar.max=100;
      bar.setAttribute('aria-label',t('Download progress'));bar.style.width='100%';
      info.append(name,detail,bar);root.append(info,actions);row={root,name,detail,bar,actions};rows.set(d.id,row);
    }
    const p=sampleProgress(d,samples.get(d.id),now);samples.set(d.id,p.sample);
    setText(row.name,(d.filename||d.url||'').split(/[\\/]/).pop());
    let detail=`${t(d.state)}${d.paused?' · '+t('Paused'):''} · ${formatBytes(p.bytes)} / ${p.total?formatBytes(p.total):t('Unknown size')}`;
    if(d.state==='in_progress'&&!d.paused){
      detail+=' · '+(p.rate===null?t('Measuring speed…'):t('{speed}/s',{speed:formatBytes(p.rate)}));
      if(p.seconds!==null)detail+=' · '+t('{seconds} seconds remaining',{seconds:Math.ceil(p.seconds)});
    }
    setText(row.detail,detail);row.bar.hidden=d.state!=='in_progress';
    if(p.percent===null)row.bar.removeAttribute('value');else row.bar.value=p.percent;
    const signature=[d.state,d.paused,d.canResume,d.exists,document.documentElement.lang].join('|');
    if(row.signature!==signature){
      row.signature=signature;row.actions.replaceChildren();row.bar.setAttribute('aria-label',t('Download progress'));
      if(d.state==='in_progress')row.actions.append(action(d.paused?'Resume':'Pause',()=>d.paused?chrome.downloads.resume(d.id):chrome.downloads.pause(d.id)),action('Cancel',()=>chrome.downloads.cancel(d.id)));
      if(d.state==='interrupted'&&d.canResume)row.actions.append(action('Resume',()=>chrome.downloads.resume(d.id)));
      if(d.state==='complete'&&d.exists)row.actions.append(action('Open',()=>chrome.downloads.open(d.id)),action('Show folder',()=>chrome.downloads.show(d.id)));
    }
  }
  // Preserve existing nodes during progress updates; reorder only when needed.
  if(items.length){
    if(!rows.size||$('items').firstChild?.nodeType===3)$('items').replaceChildren();
    items.forEach((d,index)=>{const node=rows.get(d.id).root;if($('items').children[index]!==node)$('items').insertBefore(node,$('items').children[index]||null);});
  }else setText($('items'),t('No downloads yet.'));
  if(items.some(d=>d.state==='in_progress'&&!d.paused))poll=setTimeout(schedule,1000);
}
const schedule=scheduleRender(render,e=>message($('status'),e));
$('folder').onclick=()=>chrome.downloads.showDefaultFolder();$('refresh').onclick=schedule;
chrome.downloads.onChanged.addListener(schedule);chrome.downloads.onCreated.addListener(schedule);chrome.downloads.onErased.addListener(schedule);
document.addEventListener('visibilitychange',()=>{clearTimeout(poll);samples.clear();if(!document.hidden)schedule();});
window.addEventListener('pagehide',()=>clearTimeout(poll));
document.addEventListener('maen-language',schedule);
schedule();
