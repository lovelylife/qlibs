
/*-------------------------------------------------------
 * draging.js
 * date: 2012-10-08
 * author: Q
 * powered by wayixia.com
---------------------------------------------------------*/

Q.draging = Q.extend({
  hCaptureWnd : null,
  is_drag : false,
  x : 0,
  y : 0,
  begin_left : 0,
  begin_top : 0,
  MouseDown_Hanlder : null,
  MouseUp_Handler : null,
  MouseMove_Handler : null,
  is_moved : false,
  tmr : null,
  __init__ : function(){
    var _this = this;

    // 缓存时间
    _this.MouseDown_Hanlder = function(evt) { _this._MouseDown(evt); }
    _this.MouseUp_Handler = function(evt) { _this._MouseUp(evt); }
    _this.MouseMove_Handler = function(evt) { _this._MouseMove(evt); }

    Q.addEvent(document, 'mousedown', _this.MouseDown_Hanlder);
    Q.addEvent(document, 'mouseup', _this.MouseUp_Handler);
  },

  attach_object : function(obj_or_id, config) {
    var obj = Q.$(obj_or_id);
    var config = config || {};
    obj.setAttribute('q-drag-object', true);
    obj.q_drag_objects = new Q.LIST();
    obj.q_onmove = config.onmove || function(x, y) {
      var obj = this;    
      // Q.printf('x: ' + x + '; y:' + y + ';');
      obj.style.left = x + 'px'; 
      obj.style.top  = y + 'px'; 
    };
    if(!!config.self) 
      this.add_drag_handler(obj, obj);
    var init_drag_objects = config.objects || [];
    for(var i=0; i < init_drag_objects.length; i++) {
      this.add_drag_handler(obj, init_drag_objects[i]);
    }
  },

  deattach_object : function(obj_or_id) {
    var obj = Q.$(obj_or_id);
    obj.removeAttribute('q-drag-object');
  },

  add_drag_handler : function(drag_object, handler) {
    drag_object.q_drag_objects.append(handler); 
  },
  
  remove_drag_handler : function(drag_object, handler) {
    drag_object.q_drag_objects.erase(handler); 
  },
  
  is_drag_handler : function(drag_object, handler) {
    return this.is_dragable(drag_object) && drag_object.q_drag_objects.find(handler); 
  },

  is_dragable : function(drag_object) {
    var obj = Q.$(drag_object);
    return obj && obj.getAttribute && !!obj.getAttribute('q-drag-object'); 
  },

  _MouseDown : function(evt) {
    var _this = this;
    evt = evt || window.event;
    if(evt.button == Q.RBUTTON){ return; } // 屏蔽右键拖动
    var target_wnd = drag_handle = Q.isNS6() ? evt.target : evt.srcElement; // 获取鼠标悬停所在的对象句柄
    
    while(target_wnd && !_this.is_dragable(target_wnd) && (target_wnd != document.body)) {
      target_wnd = target_wnd.parentNode;
    }

    //if(target_wnd && (!$IsMaxWindow(target_wnd)) && $IsDragObject(target_wnd, oDragHandle)) {
    if(target_wnd && _this.is_drag_handler(target_wnd, drag_handle)) {
      _this.hCaptureWnd = target_wnd; 
      _this.is_drag = true; 
      _this.x = evt.clientX;
      _this.y = evt.clientY; 
      
      _this.begin_left = target_wnd.offsetLeft;  
      _this.begin_top = target_wnd.offsetTop; 
      // 添加MouseMove事件
      _this.tmr = setTimeout(function() { Q.addEvent(document, 'mousemove', _this.MouseMove_Handler);  }, 100);
      return false; 
    }
  },
    
  _MouseMove : function(evt){
    var _this = this;
    _this.is_moved = true;
    evt = evt || window.event
    if (_this.is_drag) {
      var x = evt.clientX-_this.x;
      var y = evt.clientY-_this.y;
      if(_this.hCaptureWnd.style.zoom) {
        _this.hCaptureWnd.q_onmove(_this.begin_left+(x/_this.hCaptureWnd.style.zoom), _this.begin_top+(y/_this.hCaptureWnd.style.zoom));
      } else {
        _this.hCaptureWnd.q_onmove(_this.begin_left+x, _this.begin_top+y);
      }
      return false; 
    }
  },

  _MouseUp : function(evt) {
    var _this = this;
    clearTimeout(_this.tmr);
    if(_this.is_drag ) {
      _this.is_drag=false;
      Q.removeEvent(document,'mousemove',_this.MouseMove_Handler);
    }
    _this.is_moved=false;
  },
});

Q.Ready(function() {
  Q.drag = new Q.draging();
}, true);

