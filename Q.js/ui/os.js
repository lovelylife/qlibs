/*------------------------------------------------------------------------------------
 $ class os
 $ date: 2015-1-10 16:31
 $ author: LovelyLife http://jshtml.com
 
 $ bugs Fixed:
--------------------------------------------------------------------------------------*/

// ui template class 
Q.ui = Q.extend({
ui_iframe: null,
__init__: function(json) {
  json = json || {};
  this.ui_iframe = document.createElement("IFRAME");
  this.ui_iframe.src=json.src;
  this.ui_iframe.onload = function() {    
    json.oncomplete(true);
  }
  
  this.ui_iframe.onerror= function() {
    json.oncomplete(false);
    document.body.removeChild(this);
  }
  this.ui_iframe.style.display = "none";
  this.ui_iframe.src=json.src;
  document.body.appendChild(this.ui_iframe);
},

template: function(id) {
  var doc = this.ui_iframe.contentDocument || this.ui_iframe.contentWindow.document;
  var tpl = doc.getElementById(id);
  if(tpl)
    return tpl.cloneNode(true);

  return null;
},

bind_css : function() {
  // get ui style
  var doc = this.ui_iframe.contentDocument || this.ui_iframe.contentWindow.document;
  var sheet =doc.styleSheets[0];
  if(!sheet) // no <style>
    return;
  var cssText = sheet.ownerNode.innerHTML;
  var style=document.createElement("style");
	style.setAttribute("type", "text/css");
	if(style.styleSheet){// IE
		style.styleSheet.cssText = cssText;
	} else {// w3c
		var textNode = doc.createTextNode(cssText);
		style.appendChild(textNode);
	}

	var heads = document.getElementsByTagName("head");
	if(heads.length)
		heads[0].appendChild(style);
	else
		document.documentElement.appendChild(style);
},

})




var g_os_start_menu;
var g_os_setting;
var g_task_items = [];
var app_classes = {};

Q.os = Q.extend({
window_list_bar: null,
task_bar: null,
start_button: null,
skin: "",
__init__ : function(json) {
  json = json || {};
  this.on_logout = json.on_logout;
  this.window_list_bar = json.window_list_bar;
  this.task_bar = json.task_bar;
  this.start_button = json.start_button;
  this.skin = json.skin;

  // 重置可显示区域
  Q.client_size = function() {
    var max_width = document.body.clientWidth;
    var max_height = document.body.clientHeight;
    if( document.documentElement.clientWidth) 
      max_width = document.documentElement.clientWidth;
    if( document.documentElement.clientHeight) 
      max_height = document.documentElement.clientHeight;
    max_height = max_height - json.task_bar.offsetHeight;
    return  {width: max_width, height: max_height}
  }

  this._init_menu(json);
  // register window hooks
  register_hook(Q.bind_handler(this, this.wnds_hook));
},

_init_menu : function(json) {
  var _this = this; 
  g_os_start_menu = new class_menu({
    style: "os-start-menu", 
    on_popup: function(popup) {
      if(popup) {
        Q.addClass(_this.start_button, "q-active");
      } else {
        Q.removeClass(_this.start_button, "q-active");
      }
    }
  }); 


  this.start_button.onclick = function() {
    g_os_start_menu.showElement(this);      
  }

  var m1 = new class_menuitem({
    text: "系统设置",
    callback : function(menuitem) {
      if(!g_os_setting) {
        g_os_setting = new Q.Dialog({
          wstyle: json.skin,
          title: "系统设置", width:480, height:370, on_close: function() { delete g_os_setting; g_os_setting=null; }});
      }
      g_os_setting.set_content("test flash");
      g_os_setting.show(true);
      g_os_setting.center(); 
    }
  });

  var m2 = new class_menuitem({text: "程序", popup_style: "os-start-menu"});
  var m3 = new class_menuitem({type: MENU_SEPERATOR, text: ""});
 
  var m4 = new class_menuitem({
    text: "注销", 
    callback : function(menuitem) {
      setTimeout(function() { _this.on_logout()}, 300);
    }
  });
  
  g_os_start_menu.addMenuItem(m1);
  g_os_start_menu.addMenuItem(m2);
  g_os_start_menu.addMenuItem(m3);
  g_os_start_menu.addMenuItem(m4);

  // init applications menus
  for(var i=0; i < json.apps.length; i++) {
    var app = json.apps[i];
    var m2x = new class_menuitem({
      text: app.name,
      callback : (function(app_info) { return function(menuitem) {
        _this.run(app_info);
      }})(app),
    });
    m2.addSubMenuItem(m2x);
  }

  g_os_start_menu.hide();
},

wnds_hook : function(hwnd, message_id) {
  switch(message_id) {
  case MESSAGE.CREATE:
    {
      var _this = this;
      var button = document.createElement('button');
      button.className = "item";
      button.innerText = $GetTitleText(hwnd);  
      button.bind_data = hwnd;  
      button.onclick = function(wnd, btn){ 
        return function() {
          var nodes = _this.window_list_bar.childNodes;
          for(var i=0; i < nodes.length; i++) {
            var item = nodes[i];
            item.className = "item";
          }
          btn.className = "selected-item";
          $BindWindowMessage(wnd, MESSAGE.ACTIVATE)();
        }; 
      }(hwnd, button);
      this.window_list_bar.appendChild(button);
    }
    break;
  case MESSAGE.ACTIVATE:
    {
      var nodes = this.window_list_bar.childNodes;
      for(var i=0; i < nodes.length; i++) {
        var item = nodes[i];
        if(item.bind_data == hwnd) {
          item.className="selected-item";
        } else {
          item.className = "item";
        }
      }
    }
    break;

  case MESSAGE.CLOSE:
    {
      var nodes = this.window_list_bar.childNodes;
      for(var i=0; i < nodes.length; i++) {
        var item = nodes[i];
        if(item.bind_data == hwnd) {
          item.onclick = null;
          item.parentNode.removeChild(item);
        }
      }
    }
    break;
  } // end switch
},  // function wnds_hook

run_error: function (app, err) {
  var dialog = new Q.Dialog({
    wstyle: this.skin + " q-attr-with-bottom",
    width: 500, height: 200,
    title: "ERROR",
    content: "<div style=\"margin: 20px;line-height: 30px;word-break:break-all;\">"+err+"</div>",
    buttons: [
      {text: "关闭", onclick: function() { return true; }}
    ]
  });

  dialog.show(true);
  dialog.center();
},

// run app
run :function (app) {
  var _this = this;
  var err = "Application [" + app.name + "] is run failed.";
  app_class = app_classes[app.src];
  if(app_class) {
    // app class have loaded
    console.log("create app ok");
    try {
      new app_class({ui:app.ui_runtime});
    } catch(e) {
      _this.run_error(app, err + "<br>" + e.description);
    }
    return;
  }
  window._entry = function(t) {
    app_class = t;
  }
  Q.load_module(app.src, function(ok) {
    if(!ok) {
      _this.run_error(app, err + "<br>" + " File("+app.src+") is error.");
    } else {
      console.log("load from file and create app ok");
      app_classes[app.src] = app_class;
      // load ui
      app.ui_runtime = new Q.ui({src: app.ui, oncomplete: function(ok) {
        // init app instance
        console.log("load ui -> " + (ok?"ok":"failed"));
        try {
          new app_class({ui:app.ui_runtime});
        } catch(e) {
          _this.run_error(app, err + "<br>" + e.description);
        }
      }})
    }
  });
}

});



