/*------------------------------------------------------------------------------------
 $ class contextmenu component
 $ date: 2009-5-10 16:31
 $ author: LovelyLife http://onlyaa.com
 
 $ bugs Fixed:
--------------------------------------------------------------------------------------*/


var MENU_SEPERATOR = -1;
var MENU_ITEM = 0;
var MENU_ITEM_CHECKBOX = 1;
var MENU_ITEM_RADIO = 3;

var class_menuitem = Q.extend({
	hwnd : null,
	parentMenu : null,
	topMenu : null,
	titlewnd : null,
	subwnd : null,
	arrowwnd : null,
	iconwnd : null,
	activeItem : null,
	type : -2,
	isHidden : false,
	isAjust : false,
	clickHidden : true,
	items : null,
	binddata : null,
	isChecked : true,
	__init__ : function(json) {
		var _this = this;
		_this.items = [];
		_this.parentMenu = json.parentMenu;
		if(typeof json.type == undefined) {
			_this.type = MENU_ITEM;
		} else {
			_this.type = json.type;
		}
		_this.binddata = json.data;
		_this.initview(json);
	},
	
	initview : function(json) {
		var _this = this;
		_this.hwnd = document.createElement('li');
		_this.hwnd.style.cssText = 'width: 100%; white-space: nowrap;height: 24px; padding: 0px; margin: 0px; display: block; cursor: default; border: 0px solid blue;';
		//_this.hwnd.className = 'contextmenu menuitem';
		if(_this.type == MENU_SEPERATOR) {
			_this.hwnd.style.height = '6px';
			_this.titlewnd = document.createElement('DIV');
			_this.titlewnd.style.cssText = 'float: left; display:block; width: 100%; height: 1px; background: #c3daf9; line-height: 1px; margin:3px 3px 3px 36px ; padding: 0px;filter: alpha(opacity=80)';
			_this.hwnd.appendChild(_this.titlewnd);
		} else {
			var table = document.createElement('table');
			_this.titlewnd = table.insertRow(-1);
			_this.iconwnd = _this.titlewnd.insertCell(-1);
			var title = _this.titlewnd.insertCell(-1);
			_this.arrowwnd = _this.titlewnd.insertCell(-1);
			table.width = '100%';
			table.cellSpacing = 0;
			table.cellPadding = 0;
			table.border = 0;
			_this.hwnd.appendChild(table);
			_this.iconwnd.style.width = '35px';
			_this.arrowwnd.style.width = '35px';
			
			_this.iconwnd.innerHTML = '&nbsp;';
			title.innerHTML = json.text;
			_this.arrowwnd.innerHTML = '&nbsp;';
			
			title.style.cssText = 'width: 100%; text-algin: left; height:24px; font-size: 14px;';
			/*
			_this.titlewnd = document.createElement('div');
			_this.arrowwnd = document.createElement('div');

			_this.titlewnd.innerHTML = '&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;' + json.text;
			
			_this.titlewnd.style.cssText = 'float: left; width: auto; height: 20px; margin: 0px; padding: 3px 0px 2px 0px; color: #000; font-size: 14px; text-decoration: none; background: url(t.gif) no-repeat 5px 3px;';
			_this.arrowwnd.style.cssText = 'float: right; width: 20px; border: 1px solid; height: 100%;';
			*/
			if(_this.type == MENU_ITEM_CHECKBOX) {
				_this.isChecked = !!json.checked;
				json.icon = (_this.isChecked) ? 'checked' : 'unchecked';
				
				if(typeof json.clickHidden != 'undefined') {
					_this.clickHidden = !!json.clickHidden;
				}
			}

			if(json.icon) {
				//_this.iconwnd.style.background = 'url('+CONTEXTMENU_IMAGEPATH+'/'+json.icon+') no-repeat center center';
				_this.iconwnd.className = json.icon;
			}
		}

		// initialize event callback
		_this.hwnd.onmouseover = function(evt) {
			evt = evt || event;
			if(_this.type == MENU_SEPERATOR) { return; }
			
			if(_this.parentMenu) {
				var activeItem = _this.parentMenu.activeItem;
				if(activeItem) {
					activeItem.titlewnd.style.backgroundColor = '';
					activeItem.titlewnd.style.color = '#000';
					activeItem.hidePopup();
				}
			}
			_this.parentMenu.activeItem = _this;
			_this.titlewnd.style.backgroundColor = '#047AEC'; 
			_this.titlewnd.style.color = '#FFF';
			_this.showPopup();
		}
		
		_this.hwnd.onmouseout = function(evt) {
			evt = evt || event;
			if(_this.type == MENU_SEPERATOR) { return; }
			if(!_this.isHidden) { return; }
			_this.titlewnd.style.backgroundColor = '';
			_this.titlewnd.style.color = '#000';
			//_this.titlewnd.style.filter = 'alpha(opacity=100)';
			_this.hidePopup();
		}
		
		_this.hwnd.onmouseup = function(evt) {
			evt = evt || event;
			if(_this.subwnd) { return; }
			var callback = (typeof json.callback == 'function') ? json.callback : function(e){};
			if(callback(_this) == 0) { return; };
			var isHideTop = true;
			if(_this.type == MENU_ITEM_CHECKBOX) {
				_this.isChecked = !_this.isChecked;
				icon = (_this.isChecked) ? 'checked' : 'unchecked';
				//_this.iconwnd.style.backgroundImage = 'url('+CONTEXTMENU_IMAGEPATH+'/'+icon+')';
				_this.iconwnd.className = icon;
				if(!_this.clickHidden)
					isHideTop = false;
			}

			if(isHideTop) { _this.topMenu.hide(); }
		}
		
		_this.hwnd.oncontextmenu = function(){ return false; }
		_this.hwnd.onselectstart = function(){ return false; }
	},
	
	addSubMenuItem : function(subItem) {
		var _this = this;
		if(!subItem) return;
		if((_this.type == MENU_SEPERATOR) || (_this.type == MENU_ITEM_CHECKBOX) ) { return; }
		if(!_this.subwnd) {
			// _this.titlewnd.href = 'javascript: void(0);';
			_this.subwnd = document.createElement('ul');
			document.body.appendChild(_this.subwnd);
			//_this.subwnd.style.cssText = 'z-index: 100; width: 150px; margin: 0px 0px 0px -2px; padding-left: 0px; position: absolute; list-style: none; background: #F8F8F8 url('+CONTEXTMENU_IMAGEPATH+'/menuleft.gif) repeat-y left top; border: 1px outset #999;';
			_this.subwnd.className = 'contextmenu subwnd';
			//_this.hwnd.style.background = 'url('+CONTEXTMENU_IMAGEPATH+'/moreitem.gif) no-repeat center right';
			_this.hwnd.className = 'more';
			_this.subwnd.onmouseout = function() {
				if(_this.activeItem) {
					if(!_this.activeItem.subwnd) {
						_this.activeItem.titlewnd.style.backgroundColor = '';
						_this.activeItem.titlewnd.style.color = '#000';
					}
				}
			}
			_this.subwnd.onmousedown = function(evt) { 
				evt = evt || event;
				evt.cancelBubble = true;
								
			}
			_this.subwnd.oncontextmenu = function() { return false; }
		}
		
		_this.subwnd.appendChild(subItem.hwnd);
		subItem.parentMenu = _this;
		subItem.topMenu = _this.topMenu;
		_this.items.push(subItem);
	},
	
	hidePopup : function() {
		var _this = this;
		if(!_this.subwnd) { return; }
		if(_this.activeItem) { 
			_this.activeItem.titlewnd.style.backgroundColor = '';
			_this.activeItem.titlewnd.style.color = '#000';
			_this.activeItem = null;
		}
		_this.subwnd.style.display = 'none';
		_this.titlewnd.style.backgroundColor = '';
		_this.titlewnd.style.color = '#000';
		var len = _this.items.length;
		for(var i=0; i < len; i++) {
			_this.items[i].hidePopup();	
		}
	},
	
	showPopup : function() {
		var _this = this;
		if(!_this.subwnd) { return; }
		var pos = Q.absPosition(_this.hwnd);
		_this.subwnd.style.display = '';
		var left=0, top=0;
		if((pos.left+pos.width + _this.subwnd.offsetWidth) > document.body.clientWidth) {
			left = pos.left - _this.subwnd.offsetWidth + 2;	
		} else {
			left = pos.left + pos.width;
		}
		_this.subwnd.style.left = left + 'px';
		_this.subwnd.style.top = pos.top + 'px';
	
		if(!_this.isAjust) {
			_this.isAjust = true;
			var childNodes = _this.subwnd.childNodes;
			var len = childNodes.length;
			for(var i=0; i < len; i++) {	
				var node = childNodes[i];
				node.style.width = (_this.subwnd.offsetWidth - 2) + 'px';
			}
			//_this.subwnd.style.width = (_this.subwnd.offsetWidth + 69) + 'px';
		}
	},
	
	data : function() {
		return this.binddata;	
	},
	
	t : function() {}
});

var class_menu = Q.extend({
	hwnd : null,
	timer : null,
	isajust : false,
	activeItem : null,
	items : null,
	_fHide : null,
	__init__ : function() {
		var _this = this;
		_this.items = [];
		_this.initview();
		_this._fHide = function() { _this.hide(); }
	},
	
	initview : function() {
		var _this = this;
		_this.hwnd = document.createElement('DIV');
		_this.hwnd.style.cssText = 'display:none;'
		document.body.appendChild(_this.hwnd);

		_this.hwnd.onmousedown = function(evt){
			evt = evt || window.event;
			evt.cancelBubble = true;
		}
		
		_this.hwnd.onmouseout = function() {
			if(_this.activeItem) {
				if(!_this.activeItem.subwnd) {
					_this.activeItem.titlewnd.style.backgroundColor = '';
					_this.activeItem.titlewnd.style.color = '#000';
				}
			}
		}

		Q.addEvent(document, 'mousedown', _this._fHide);
//		if(document.all) {
//			Q.addEvent(_this.hwnd, "losecapture", _this._fHide);
//		} else {
//			Q.addEvent(window, "blur", _this._fHide);
//		}
	},
	
	addMenuItem : function(item) {
		var _this = this;
		_this.hwnd.appendChild(item.hwnd);
		item.parentMenu = _this;
		item.topMenu = _this;
		_this.items.push(item);
	},
	
	show : function(){
		var _this = this;
		var scroll = Q.scrollInfo();
		var left = 0, top = 0;
		if(_this.hwnd.className == '') {
			_this.hwnd.className = 'contextmenu';
		}

		_this.hwnd.style.display = '';
		if((event.clientX + _this.hwnd.offsetWidth) > document.body.clientWidth)
		    left = event.clientX  + scroll.l - _this.hwnd.offsetWidth - 2;
		else
		    left = event.clientX + scroll.l;
		
		if( (event.clientY + _this.hwnd.offsetHeight) > document.body.clientHeight)
		    top = event.clientY  + scroll.t - _this.hwnd.offsetHeight - 2;
		else
		    top = event.clientY + scroll.t;
		
		_this.hwnd.style.left = left+'px';
		_this.hwnd.style.top = top +'px';
		document.onmousewheel = function() {	return false;	}
		if(!_this.isajust) {
			_this.isajust = true;
			var childNodes = _this.hwnd.childNodes;
			var len = childNodes.length;
			for(var i=0; i < len; i++) {	
				var node = childNodes[i];
				node.style.width = (_this.hwnd.offsetWidth - 2) + 'px';
			}
		}

		

		if(document.all) {
			Q.addEvent(window, "blur", _this._fHide);
			//Q.addEvent(_this.hwnd, "mouseup", _this._fHide);
			Q.addEvent(document, "mouseup", _this._fHide);
		} else {
			Q.addEvent(window, "blur", _this._fHide);
			Q.addEvent(document, "mouseup", _this._fHide);
		}
	},
	
	showElement : function(element, isClosed) {
		var _this = this;
		if(_this.hwnd.className == '') {
			_this.hwnd.className = 'contextmenu'
		}
		_this.hide();
		if(element.nodeType != 1) { return; }
		_this.hwnd.style.display = '';
		if(!this.isajust) {
			this.isajust = true;
			var childNodes = this.hwnd.childNodes;
			var len = childNodes.length;
			for(var i=0; i < len; i++) {	
				var node = childNodes[i];
				node.style.width = (_this.hwnd.offsetWidth - 2) + 'px';
			}
			//_this.hwnd.style.width = (_this.hwnd.offsetWidth + 69) + 'px';
		}
		var pos = Q.absPosition(element);
		
		var left =0, top = 0;
		top = pos.top + pos.height;
		
		if(_this.hwnd.offsetWidth + pos.left > document.body.clientWidth) {
			left = pos.left + pos.width - _this.hwnd.offsetWidth;	
		} else {
			left = pos.left;	
		}
		
		_this.hwnd.style.left = left + 'px';
		_this.hwnd.style.top = top + 'px';

//		if(document.all) {
//			Q.addEvent(_this.hwnd, "losecapture", _this._fHide);
//		} else {
//			Q.addEvent(window, "blur", _this._fHide);
//		}
		
	},
	
	hide : function(){
		var _this = this;
		if(document.all) {
			Q.removeEvent(document, "mouseup", _this._fHide);
		} else {
			Q.removeEvent(window, "blur", _this._fHide);
		}

		document.onmousewheel = null;
		_this.hwnd.style.display = 'none';
		var len = _this.items.length;
		for(var i=0; i < len; i++) {
			_this.items[i].hidePopup();
		}
	}
});
