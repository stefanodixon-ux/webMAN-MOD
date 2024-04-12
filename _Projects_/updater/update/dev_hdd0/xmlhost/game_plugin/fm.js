function tg(b,m,x,c){
	var i,p,o,h,l=document.querySelectorAll('.d,.w'),s=m.length,n=1;
	for(i=1;i<l.length;i++){
		o=l[i];
		h=o.href;p=h.indexOf('/cpy.ps3');if(p>0){n=0;s=8;bCpy.value='Copy';}
		if(p<1){p=h.indexOf('/cut.ps3');if(p>0){n=0;s=8;bCut.value='Cut';}}
		if(p<1){p=h.indexOf('/delete.ps3');if(p>0){n=0;s=11;bDel.value='Delete';}}
		if(p>0){o.href=h.substring(p+s,h.length);o.style.color='#ccc';}
		else{p=h.indexOf('/',8);o.href=m+h.substring(p,h.length);o.style.color=c;}
	}
	if(n)b.value=(b.value == x)?x+' Enabled':x;
}

// F2 = rename/move item pointed with mouse
document.addEventListener('keyup',ku,false);

function rn(f){
	if(f.substring(0,5)=='/dev_'){
		f=unescape(f);
		t=prompt('Rename to:',f);
		if(t&&t!=f)self.location='/rename.ps3'+f+'|'+escape(t)
	}
}
function ku(e){
	e=e||window.event;
	if(e.keyCode==113){try{a=document.querySelectorAll('a:hover')[0].pathname.replace('/mount.ps3','');rn(a);}catch(err){}}
	$('mnu').style.display='none';
}

// Right-click menu
document.write( "<div id='mnu' style='position:fixed;width:180px;background:#333;z-index:9;display:none;padding:5px;box-shadow:3px 3px 6px #222;opacity:0.96'>"+
				"<a id='m0'>Install PKG</a>"+
				"<a id='m1'>Mount</a>"+
				"<a id='m2'>Open<br></a>"+
				"<a id='v2'>Hex View<br></a>"+
				"<a id='ms'>Game Info<br></a>"+
				"<hr>"+
				"<a id='m3'>Delete<br></a>"+
				"<a id='m7'>Rename<br></a>"+
				"<a id='mf' href=\"javascript:t=prompt('New Folder', self.location.pathname);if(t.indexOf('/dev_')==0)self.location='/mkdir.ps3'+t\">New Folder</a>"+
				"<a id='dm'>Truncate</a><br>"+
				"<a id='md'>MD5 Hash</a><br>"+
				"<hr>"+
				"<a id='m4'>Cut<br></a>"+
				"<a id='m5'>Copy<br></a>"+
				"<a id='m6'>Paste<br></a>"+
				"<hr>"+
				"<a id='ml'>Dir by Name<br></a>"+
				"<a id='ss'>Dir by Size<br></a>"+
				"<a id='sd'>Dir by Date<br></a>"+
				"<hr>"+
				"<a id='mz'>Zip Folder<br></a>"+
				"<a id='m8'>Copy To</a><br>"+
				"<a id='m9'>Copy & overwrite<br></a>"+
				"</div>");

var s,m;

document.oncopy = function(e){
	e.preventDefault();
	try{a=document.querySelectorAll('a:hover')[0].href;}catch(err){}
	var clipboard=e.clipboardData;
	clipboard.setData("text/plain",a);
};

function contextMenu(e){
	if(s)s.color='#ccc';
	t=e.target,s=t.style,c=t.className,m=mnu.style;if(c=='gi'){p=t.parentNode.pathname}else{p=t.pathname}if(typeof(p)!='string')return;p=p.replace('/mount.ps3','');
	if(c.charAt(0)=='w'||c=='w'||c=='d'||c=='gi'||t.parentNode.className=='gn'){
		e.preventDefault();
		s.color='#fff',b='block',n='none';
		m.display=b;
		m.left=(e.clientX+12)+'px';pp=p.toLowerCase();
		y=(pp.indexOf('.pkg')>0);w=(pp.indexOf('.iso')>0);
		img=(pp.indexOf('.png')>0)||(pp.indexOf('.jpg')>0)||(pp.indexOf('.gif')>0);
		m0.href='/install.ps3'+p;m0.style.display=y?b:n;
		m1.href='/mount.ps3'+p;m1.style.display=!img&&!y&&(w||c=='d'||p.indexOf('/GAME')>0||p.indexOf('ISO/')>0||p.indexOf('ISO_')>0||p.indexOf('/ROMS')>0)?b:n;
		m2.href=p;m2.text=(w||c=='w')?'Download':'Open';
		v2.href='/view.ps3'+p;v2.style.display=(c!='d')?b:n;
		exts=/(\.txt|\.ini|\.log|\.ncl|\.sfx|\.xml|\.cfg|\.cnf|\.his|\.hip|\.bup|\.js|\.css|\.htm|\.bat|\.yaml|\.conf)$/i;if(exts.exec(p)){v2.href='/edit.ps3'+p;v2.text='Edit';}
		m3.href='/delete.ps3'+p;
		m4.href='/cut.ps3'+p;
		m5.href='/cpy.ps3'+p;
		m6.href='/paste.ps3'+self.location.pathname;m6.style.display=mf.style.display=(c=='w'||c=='d')?b:n;
		m7.href='javascript:rn(\"'+p+'\")';m7.style.display=(p.substring(0,5)=='/dev_')?b:n;
		m8.href='/copy.ps3'+p; m8.text = 'Copy to ' + ((p.indexOf('/dev_hdd')==0) ? "usb" : "hdd0");
		m9.href='/copy_ps3'+p;
		mz.href=((c=='d')?'/dozip.ps3':'/unzip.ps3')+p;
		mz.text=((c=='d')?'Zip Folder':'Unzip');exts=/(\.zip|\.rar|\.7z|\.bz2|\.tar|\.tgz)$/i;
		mz.style.display=(c=='d')?b:(exts.exec(p)?b:n);
		md.href=((c=='d')?'/stat.ps3':'/md5.ps3')+p;
		md.text=((c=='d')?'Folder Size':'MD5 Hash');
		dm.href=((c=='d')&&!p.includes('/USRDIR/')?'/chmod.ps3':'/trunc.ps3')+p;
		dm.text=((c=='d')&&!p.includes('/USRDIR/')?'Fix Permissions':'Truncate');
		ms.href='http://google.com/search?q='+t.text;ms.style.display=(t.parentNode.className=='gn')?b:n;
		y=p.indexOf('.ps3');if(y>0)p=p.substring(y+4);url=window.location.href;
		ml.href=p.substring(0,p.lastIndexOf("/"));if(url.indexOf('?sort=')<0)ml.href+='?sort=name&desc';
		ss.href=p.substring(0,p.lastIndexOf("/"))+'?sort=size';if(url.indexOf('?sort=size')>0 && url.indexOf('&desc')<0)ss.href+='&desc';
		sd.href=p.substring(0,p.lastIndexOf("/"))+'?sort=date';if(url.indexOf('?sort=date')>0 && url.indexOf('&desc')<0)sd.href+='&desc';
		y=e.clientY;w=window.innerHeight;m.top=(((y+mnu.clientHeight)<w)?(y+12):(w-mnu.clientHeight))+'px';
	}
}

window.addEventListener('contextmenu',contextMenu,false);

// Clear menu
var tapHold, tapEvent, tapEventFired = false;
window.onclick=function(e){if(tapEventFired){tapEvent.preventDefault();tapEvent.stopPropagation();return false;}if(m)m.display='none';wmsg.style.display='none';t=e.target;if(t.id.indexOf('im')==0||(typeof(t.href)=='string'&&t.href.indexOf('.ps3')>0&&t.href.indexOf('view.ps3')<0&&t.href.indexOf('.png')<0&&t.href.indexOf('.jpg')<0&&t.href.indexOf('prompt')<0&&t.href.indexOf('#Top')<0))wmsg.style.display='block';}

document.addEventListener('mousedown', function(event){
	if(event.target.id!='sz')
		event.preventDefault();
	tapEvent = event;
	tapEventFired = false;
	tapHold = setTimeout(function(e){
		tapEventFired = true;
		tapEvent.stopPropagation();
		contextMenu(tapEvent);
		clearTimeout(tapHold);
	}, 2000);
},false);

document.addEventListener('mouseup', function(event){
	clearTimeout(tapHold);
	if(tapEventFired) {event.preventDefault();event.stopPropagation();return false;}
},false);
