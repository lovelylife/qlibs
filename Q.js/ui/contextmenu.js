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

Q.workspace = function() {
 var max_height = document.body.clientHeight;
  if( document.documentElement.clientHeight) {
    max_height = document.documentElement.clientHeight;
  }

  var max_width = document.body.clientWidth;
  if( document.documentElement.clientWidth) {
    max_width = document.documentElement.clientWidth;
  }

  return  {width: max_width, height: max_height}
} 

var class_menuitem = Q.extend({
hwnd : null,
parentMenu : null,
topMenu : null,
titlewnd : null,
subwnd : null,
iconwnd : null,
activeItem : null,
type : -2,
isAjust : false,
clickHidden : true,
items : null,
binddata : null,
isChecked : true,
popup_style: null,
__init__ : function(json) {
  json = json || {};
  var _this = this;
  _this.items = [];
  _this.parentMenu = json.parentMenu;
  _this.binddata = json.data;
  this.popup_style = json.popup_style;
  this.type = json.type || MENU_ITEM; 
  // construct dom
  _this.hwnd = document.createElement('LI');
  _this.titlewnd = document.createElement('DIV');
  _this.hwnd.appendChild(_this.titlewnd);
  if(json.type == MENU_SEPERATOR) {
    _this.hwnd.className = 'q-item-seperator';
    _this.titlewnd.className = "q-line"
  } else {
    _this.hwnd.className = 'q-item';
    _this.titlewnd.className = "q-item-title";
    _this.iconwnd = document.createElement("button");
    _this.iconwnd.className = "q-item-icon";
    _this.iconwnd.innerHTML = "&nbsp;";
    _this.titlewnd.appendChild(_this.iconwnd);
    _this.titlewnd.appendChild(document.createTextNode(json.text+"\u00A0\u00A0\u00A0\u00A0"));
    
    if(json.icon)
      _this.iconwnd.style.background = json.icon;

    // initialize event callback
    _this.hwnd.onmouseover = function(evt) {
      evt = evt || event;
      if(_this.parentMenu) {
        var activeItem = _this.parentMenu.activeItem;
        if(activeItem) {
          activeItem.hidePopup();
        }
      }
      _this.parentMenu.activeItem = _this;
      _this.showPopup();
    }
  
    _this.hwnd.onmouseout = function(evt) {
      evt = evt || event;
      if(Q.hasClass(_this.hwnd, "q-active")) { 
        return;
      }
      _this.hidePopup();
    }
  
    _this.hwnd.onmouseup = function(evt) {
      evt = evt || event;
      if(_this.subwnd) { 
        return; 
      }
      var callback = (typeof json.callback == 'function') ? json.callback : function(e){};
      if(callback(_this) == 0)  
        return; 
      var isHideTop = true;
      if(isHideTop) 
        _this.topMenu && _this.topMenu.hide(); 
    }
  }
  
  _this.hwnd.oncontextmenu = function(evt) { return false; }
  _this.hwnd.onselectstart = function(evt) { return false; }
},

addSubMenuItem : function(subItem) {
  if((this.type == MENU_SEPERATOR)
   || (this.type == MENU_ITEM_CHECKBOX))
  {
    return;
  }
  if(!this.subwnd) 
  {
    this.subwnd = document.createElement("DIV");
    document.body.appendChild(this.subwnd);
    this.subwnd.className = 'q-contextmenu';
    if(this.popup_style)
      Q.addClass(this.subwnd, this.popup_style);

    Q.addClass(this.hwnd, 'q-more');
    this.subwnd.onmousedown = function(evt) { 
      evt = evt || event;
      evt.cancelBubble = true;
    }
    this.subwnd.oncontextmenu = function(evt) { return false; }
  }
  
  this.subwnd.appendChild(subItem.hwnd);
  subItem.parentMenu = this;
  subItem.topMenu = this.topMenu;
  this.items.push(subItem);
},

hidePopup : function() {
  if(!this.subwnd) 
    return

  Q.removeClass(this.hwnd, "q-active");
  if(this.activeItem) { 
    this.activeItem = null;
  }
  this.subwnd.style.display = 'none';
  var len = this.items.length;
  for(var i=0; i < len; i++) {
    this.items[i].hidePopup();  
  }
},

showPopup : function() {
  if(!this.subwnd)  
    return; 
  Q.addClass(this.hwnd, "q-active");
  this.subwnd.style.display = '';
  var workspace = Q.workspace();
  //Q.printf("subwnd width -> " + this.subwnd.offsetWidth + ":" + "subwnd height -> " + this.subwnd.offsetHeight)
  var pos = Q.absPositionEx(this.hwnd);
  var x =0, y = 0;
  if(pos.top+pos.height+this.subwnd.offsetHeight > workspace.height ) {
    //Q.printf("height overflow bottom, top: " + pos.top + ", height: " + pos.height + ", popup height: " + this.subwnd.offsetHeight);
    y = pos.top+pos.height-this.subwnd.offsetHeight;
    if(y < 0)  {
      //Q.printf("height overflow top");
      y = 0;
    }
  } else {
    y = pos.top;    
  }
  if((this.subwnd.offsetWidth + pos.left+pos.width) > workspace.width) {
    x = pos.left - this.subwnd.offsetWidth; 
    if(x < 0) 
      x = 0;
  } else {
    x = pos.left+pos.width;
    
  }
  var si = Q.scrollInfo(); 
  this.subwnd.style.left = si.l + x + 'px';
  this.subwnd.style.top = (si.t+y) + 'px';
},

data : function() {
  return this.binddata;  
},

});

var class_menu = Q.extend({
hwnd : null,
subwnd: null,
timer : null,
isajust : false,
activeItem : null,
items : null,
_fHide : null,
_fWheel: null,
_fOnPopup : null,
__init__ : function(json) {
  json = json || {};
  var _this = this;
  _this.items = [];
  _this._fHide = Q.bind_handler(this, this.hide);
  if(typeof json.on_popup == 'function') {
    this._fOnPopup = json.on_popup;
  } else {
    this._fOnPopup = function(popup) {};
  }

  Q.addEvent(document, 'mousedown', this._fHide);
  _this.initview(json);
},

initview : function(json) {
  this.hwnd = document.createElement('DIV');
  this.hwnd.className = 'q-contextmenu';
  document.body.appendChild(this.hwnd);
  Q.addClass(this.hwnd, json.style);

  this.hwnd.onmousedown = function(evt){
    evt = evt || window.event;
    evt.cancelBubble = true;
  }
  
  Q.addEvent(document, 'mousedown', this._fHide);
},

addMenuItem : function(item) {
  var _this = this;
  _this.hwnd.appendChild(item.hwnd);
  item.parentMenu = _this;
  item.topMenu = _this;
  _this.items.push(item);
},

show : function(evt){
  var _this = this;
  var scroll = Q.scrollInfo();
  var left = 0, top = 0;
  evt = evt || window.event;
  _this.hwnd.style.display = '';
  if((evt.clientX + _this.hwnd.offsetWidth) > document.body.clientWidth)
      left = evt.clientX  + scroll.l - _this.hwnd.offsetWidth - 2;
  else
      left = evt.clientX + scroll.l;
  
  if( (evt.clientY + _this.hwnd.offsetHeight) > document.body.clientHeight)
      top = evt.clientY  + scroll.t - _this.hwnd.offsetHeight - 2;
  else
      top = evt.clientY + scroll.t;
  
  _this.hwnd.style.left = left+'px';
  _this.hwnd.style.top  = top +'px';
  this._fWheel = document.onmousewheel;
  document.onmousewheel = function() { return false; }
  if(!_this.isajust) {
    _this.isajust = true;
    var childNodes = _this.hwnd.childNodes;
    var len = childNodes.length;
    for(var i=0; i < len; i++) {  
      var node = childNodes[i];
      node.style.width = (_this.hwnd.offsetWidth - 2) + 'px';
    }
  }

  Q.addEvent(window, "blur", _this._fHide);
  Q.addEvent(document, "mouseup", _this._fHide);
},

showElement : function(element, isClosed) {
  var _this = this;
  _this.hide();
  Q.addEvent(document, "mousedown", this._fHide);
  Q.addEvent(window, "blur", this._fHide);
  this._fOnPopup(true);
  if(element.nodeType != Q.ELEMENT_NODE)  
    return; 
  
  _this.hwnd.style.display = '';
  if(!this.isajust) {
    this.isajust = true;
    var childNodes = this.hwnd.childNodes;
    var len = childNodes.length;
    for(var i=0; i < len; i++) {  
      var node = childNodes[i];
      node.style.width = (_this.hwnd.offsetWidth - 2) + 'px';
    }
  }
  var workspace = Q.workspace();
  var pos = Q.absPositionEx(element);
  var left =0, top = 0;
  if(pos.top+pos.height+_this.hwnd.offsetHeight > workspace.height ) {
    top = pos.top-_this.hwnd.offsetHeight;
  } else {
    top = pos.top + pos.height;
  }
  if(_this.hwnd.offsetWidth + pos.left > workspace.width) {
    left = pos.left+pos.width - _this.hwnd.offsetWidth;  
  } else {
    left = pos.left;  
  }
  
  var si = Q.scrollInfo();
  _this.hwnd.style.left = si.l + left + 'px';
  _this.hwnd.style.top = si.t + top + 'px';
},

hide : function() {
  //Q.printf("hide context menu");
  this.hwnd.style.display = 'none';
  Q.removeEvent(window, "blur", this._fHide);
  Q.removeEvent(document, "mousedown", this._fHide);
  document.onmousewheel = this._fWheel;
  this._fOnPopup(false);

  var len = this.items.length;
  for(var i=0; i < len; i++) {
    this.items[i].hidePopup();
  }
}
});

var class_menubar = Q.extend({
hwnd: null,
focus: null,
items: null,
__init__: function(json) {
  json = json || {};
  this.items = new Q.LIST();
  this.hwnd = document.createElement('DIV');
  this.hwnd.className = "q-contextmenu q-contextmenu-bar";
  Q.$(json.container).appendChild(this.hwnd);
},

append: function(text, menu) {
  var item = document.createElement('button');
  item.className = "q-item";
  item.innerText = text;
  item.onmousedown = (function(bar, i, m) { 
    return function(evt) {
      //console.log("mousedown item")
      evt = evt || window.event;
      var obj = Q.isNS6() ? evt.target : evt.srcElement; // 获取鼠标悬停所在的对象句柄
      if((obj == i) && (bar.focus)) {
        i.blur();
        evt.returnValue = false;
        evt.cancelBubble = true;
        return false;
      }

      return true;
    } 
  })(this, item, menu);

  item.onfocus = (function(bar, i, m) { 
    return function(evt) {
      //console.log("onfocus item")
      bar.focus = true;
      if(m)
        m.showElement(i);
    } 
  })(this, item, menu);

  item.onblur = (function(bar, m) { return function(evt) {
    bar.focus = false;
    if(m)
      m.hide();
    //console.log("kill focus -> " + bar.focus);
  }})(this, menu);

  item.onmouseover = (function(bar, i) { return function(evt) {
    if(bar.focus)
      i.focus();
  }})(this, item);

  this.hwnd.appendChild(item);
  this.items.append(item);
}

})


