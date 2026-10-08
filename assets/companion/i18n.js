import {translations} from './translations.js';
let language='en';
// No top-level await: this module is also imported by the MV3 service worker.
const reverse=new Map();
for(const [key,pair] of Object.entries(translations))for(const value of pair)reverse.set(value,key);
const rendered=new Map();
export function t(key,vars={}) {
  const canonical=reverse.get(key)||key;
  let result=(translations[canonical]||[canonical,canonical])[language==='ar'?1:0];
  result=result.replace(/\{(\w+)\}/g,(_,name)=>String(vars[name]??''));
  if(Object.keys(vars).length){if(rendered.size>500)rendered.clear();rendered.set(result,{key:canonical,vars});}
  return result;
}
export function errorText(value){
  const raw=value instanceof Error?value.message:String(value);
  if(rendered.has(raw)){const spec=rendered.get(raw);return t(spec.key,spec.vars);}
  if(translations[raw]||reverse.has(raw))return t(raw);
  console.warn('MaenBrowser:',raw);
  return t('An operation failed. Please try again.');
}
export function currentLanguage(){return language;}
const records=new WeakMap();
const ignored='script,style,svg,.tab-title,.tab-url,.user-content,#text,#title,#source';
function translateText(node){
  if(node.nodeType!==3||node.parentElement?.closest(ignored))return;
  const text=node.nodeValue.trim();if(!text)return;
  let record=records.get(node);
  if(!record||record.last!==node.nodeValue){
    const spec=rendered.get(text);const key=spec?.key||reverse.get(text)||text;
    if(!translations[key])return;
    record={key,vars:spec?.vars||{},prefix:node.nodeValue.match(/^\s*/)[0],suffix:node.nodeValue.match(/\s*$/)[0]};
  }
  const translated=record.prefix+t(record.key,record.vars)+record.suffix;
  record.last=translated;records.set(node,record);
  if(node.nodeValue!==translated)node.nodeValue=translated;
}
function translateTree(root){
  if(root.nodeType===3){translateText(root);return;}
  if(root.nodeType!==1&&root.nodeType!==9)return;
  if(root.nodeType===1&&root.closest(ignored))return;
  const walker=document.createTreeWalker(root,NodeFilter.SHOW_TEXT);
  while(walker.nextNode())translateText(walker.currentNode);
  const elements=[...(root.nodeType===1?[root]:[]),...root.querySelectorAll('[placeholder],[aria-label],[title]')];
  for(const el of elements)for(const attr of ['placeholder','aria-label','title']){
    const value=el.getAttribute(attr);if(value&&reverse.has(value))el.setAttribute(attr,t(value));
  }
}
function apply(){document.documentElement.lang=language;document.documentElement.dir=language==='ar'?'rtl':'ltr';translateTree(document);}
if(typeof document!=='undefined'){
  apply();
  chrome.storage.local.get('language').then(p=>{language=p.language==='ar'?'ar':'en';apply();document.dispatchEvent(new Event('maen-language'));}).catch(()=>{});
  const observer=new MutationObserver(changes=>{
    for(const change of changes){if(change.type==='characterData')translateText(change.target);else for(const node of change.addedNodes)translateTree(node);}
  });
  observer.observe(document.documentElement,{subtree:true,childList:true,characterData:true});
  chrome.storage.onChanged.addListener((changes,area)=>{if(area==='local'&&changes.language&&changes.language.newValue!==language){language=changes.language.newValue==='ar'?'ar':'en';apply();document.dispatchEvent(new Event('maen-language'));}});
}
