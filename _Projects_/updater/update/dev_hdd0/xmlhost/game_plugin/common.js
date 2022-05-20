//document.body.style.backgroundColor = "#123456";
//document.body.style.backgroundImage = "url('/dev_hdd0/xmlhost/game_plugin/background.jpg')";
function $$(id){return document.getElementById(id);}
if(typeof $!=="function"){window.$=function(id){return document.getElementById(id);}}
function _find(){var s=prompt('Search:','');if(s){document.getElementById('rhtm').style.display='block';self.location='/index.ps3?'+escape(s)}}
