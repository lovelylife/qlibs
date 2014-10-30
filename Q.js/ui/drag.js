/*-----------------------------------------------------------------
 $ javascript dragging
 $ author: Q
 $ date: 2014-10-22
-------------------------------------------------------------------*/

Q.draging = Q.extend({
  nn6 : Q.isNS6(),
  ie  : Q.isIE(),
  hCaptureWnd : null,
  isdrag : false,
  x : 0,
  y : 0,
  beginX : 0,
  beginY : 0,
  endX : 0,
  endY : 0,
  MouseDown_Hanlder : null,
  MouseUp_Handler : null,
  MouseMove_Handler : null,
  isMoved : false,
  tmr : null,
  construct : function(){
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
      var width = obj.offsetWidth;
      var height= obj.offsetHeight;
      obj.style.left = x + 'px'; 
      obj.style.top  = y + 'px'; 
      obj.style.right = (x+width) + 'px';
      obj.style.bottom= (y+height) + 'px';  
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
    return !!obj.getAttribute('q-drag-object'); 
  },

  _MouseDown : function(evt) {
    var _this = this;
    evt = evt || window.event;
    if(evt.button == Q.RBUTTON){ return; } // 屏蔽右键拖动
    var target_wnd = drag_handle = _this.nn6 ? evt.target : evt.srcElement; // 获取鼠标悬停所在的对象句柄
    
    while(target_wnd && !_this.is_dragable(target_wnd) && (target_wnd != document.body)) {
      target_wnd = target_wnd.parentNode;
    }

    //if(target_wnd && (!$IsMaxWindow(target_wnd)) && $IsDragObject(target_wnd, oDragHandle)) {
    if(target_wnd && _this.is_drag_handler(target_wnd, drag_handle)) {
      var pos = Q.absPosition(target_wnd);
      _this.isdrag = true; 
      _this.hCaptureWnd = target_wnd; 
      _this.beginY = pos.top; //parseInt(_this.hCaptureWnd.style.top+0); 
      _this.y = _this.nn6 ? evt.clientY : evt.clientY; 
      _this.beginX = pos.left; //parseInt(_this.hCaptureWnd.style.left+0); 
      _this.x = _this.nn6 ? evt.clientX : evt.clientX;
        
      // 添加MouseMove事件
      _this.tmr = setTimeout(function() { Q.addEvent(document, 'mousemove', _this.MouseMove_Handler);  }, 100);
      return false; 
    }
  },
    
  _MouseMove : function(evt){
    var _this = this;
    _this.isMoved = true;
    evt = evt || window.event
    if (_this.isdrag) {
      var x = (_this.nn6?(_this.beginX+evt.clientX-_this.x):(_this.beginX+evt.clientX-_this.x));
      var y = (_this.nn6?(_this.beginY+evt.clientY-_this.y):(_this.beginY+evt.clientY-_this.y));
      // 移动拖动窗口位置
      var pos_parent = {left:0, top:0, right:0, bottom:0};
      if(_this.hCaptureWnd.parentNode) {
        pos_parent = Q.absPosition(_this.hCaptureWnd.parentNode);
      }
      _this.hCaptureWnd.q_onmove(x-pos_parent.left, y-pos_parent.top);
      // 保存坐标
      _this.endX = x;
      _this.endY = y;

      return false; 
    }
  },

  _MouseUp : function(evt) {
    var _this = this;
    clearTimeout(_this.tmr);
    if(_this.isdrag ) {
      var pos = Q.absPosition(_this.hCaptureWnd.parentNode);
      Q.removeEvent(document,'mousemove',_this.MouseMove_Handler);
      _this.isdrag=false;
      var x = _this.endX-pos.left;
      var y = _this.endY-pos.top;
      Q.removeEvent(document,'mousemove',_this.MouseMove_Handler);
      _this.isMoved && _this.hCaptureWnd && _this.hCaptureWnd.q_onmove(x, y);
    }
    _this.isMoved=false;
  },
});

Q.Ready(function() {
  Q.drag = new Q.draging();
}, true);

