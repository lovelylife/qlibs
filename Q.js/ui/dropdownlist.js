/*-------------------------------------------------------
  file: dropdownlist.js
  date: 2015-03-22
  author: Q
---------------------------------------------------------*/


Q.dropdownlist = Q.extend({
hwnd: null,
drop_wnd: null,
ctrl: null,
__init__: function(json) {
  var _this = this;
  this.hwnd = document.createElement('BUTTON');
  this.hwnd.className = 'q-select';
  Q.addClass(this.hwnd, json.wstyle);
  this.on_change =  json.on_change || function(text, value) {};
  this.ctrl = Q.$(json.render);
  this.ctrl.parentNode.insertBefore(this.hwnd, this.ctrl);
  this.ctrl.onchange = ( function(o, c) {
    return function(evt) {
      Q.printf("ctrl onchage");
      o.on_item_changed(c.options[c.selectedIndex].text, c.options[c.selectedIndex].value);
    }
  })(this, this.ctrl);
  var bar = new class_menubar();
  this.drop_wnd = new class_menu({
    style: json.wstyle, 
    on_popup: function(popup) {
      if(popup)
        Q.addClass(_this.hwnd, "checked");
      else
        Q.removeClass(_this.hwnd, "checked");
    }
  });
  if(this.ctrl.tagName.toLowerCase() == "select") {
    var len = this.ctrl.options.length;
    for(var i=0; i < len; i++) {
      var m4 = new class_menuitem({text: this.ctrl.options[i].text, data: i, callback: Q.bind_handler(_this, _this.on_menu_clicked)});
      this.drop_wnd.addMenuItem(m4);
      if(i == this.ctrl.selectedIndex) {
        this.set_value(this.ctrl.options[i].value);
        this.set_text(this.ctrl.options[i].text);
      }
    }
  }
  this.drop_wnd.hide(); 
  bar.append(this.hwnd, this.drop_wnd);
},

on_menu_clicked: function(menu) {
  Q.printf(menu.data);
  var index = menu.data;
  if(index == this.ctrl.selectedIndex) {
    return;
  } else {
    this.set_value(this.ctrl.options[index].value);
  }
},

on_item_changed : function(text, value) {
  this.set_text(text);
  this.on_change(text, value);
},

set_text : function(text) {
  
  if(this.trim(this.hwnd.innerText) == this.trim(text)) {
    return false;
  } else {
    this.hwnd.innerText = this.trim(text);
  }

  return true;
},

set_value : function(value) {
  var e = this.ctrl;
  var selected_index = this.ctrl.selectedIndex;
  for(var i=0;i<e.options.length;i++) {
    if(e.options[i].value == value) {
      if(selected_index != i) {
        e.options[i].selected = true;
        this.on_item_changed(e.options[i].text, e.options[i].value)
        return true;
      }
      break;
    }
  }

  return false;
},
 
trim : function(s) {
  return s.replace(/&nbsp;/, '');
},

}); // code end


