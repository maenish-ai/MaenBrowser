const input=document.getElementById('settingsQuery');
function filter(){
 if(!input)return;
 const query=input.value.trim().toLocaleLowerCase();
 for(const card of document.querySelectorAll('#fields section.card')){
  // Authentication controls and error/status information must remain reachable.
  card.hidden=!!query&&!card.querySelector('#pin')&&!card.textContent.toLocaleLowerCase().includes(query);
 }
}
input?.addEventListener('input',filter);
document.addEventListener('maen-language',filter);
