export function selectTabView(tabs,{query='',sort='position',limit=100}={}){
 const q=String(query).toLocaleLowerCase();
 let result=tabs.filter(t=>!t.incognito&&(String(t.title||'')+' '+String(t.url||'')).toLocaleLowerCase().includes(q));
 if(sort==='title')result.sort((a,b)=>String(a.title||a.url||'').localeCompare(String(b.title||b.url||'')));
 if(sort==='recent')result.sort((a,b)=>(Number(b.lastAccessed)||0)-(Number(a.lastAccessed)||0));
 const count=result.length;return {tabs:result.slice(0,Math.max(1,Math.min(500,limit))),count};
}
