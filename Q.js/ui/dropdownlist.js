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
  this.ctrl = Q.$(json.render);
  this.ctrl.parentNode.insertBefore(this.hwnd, this.ctrl);
  
  var bar = new class_menubar();
  this.drop_wnd = new class_menu({style: json.wstyle});
  if(this.ctrl.tagName.toLowerCase() == "select") {
    var len = this.ctrl.options.length;
    for(var i=0; i < len; i++) {
      var m4 = new class_menuitem({text: this.ctrl.options[i].text, callback: Q.bind_handler(_this, _this.on_item_changed)});
      this.drop_wnd.addMenuItem(m4);
    }
  }
  this.drop_wnd.hide(); 
  bar.append(this.hwnd, this.drop_wnd);
},

  on_item_changed: function(m) {
    this.hwnd.innerText = m.hwnd.innerText; 
  },




}); // code end


