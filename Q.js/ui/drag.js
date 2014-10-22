/*-----------------------------------------------------------------
 $ javascript dragging
 $ author: Q
 $ date: 2014-10-22
-------------------------------------------------------------------*/

Q.draging = Q.extend({
  nn6 : Q.isNS6(),
  ie  : Q.isIE(),
  hCaptureWnd : null,
  //hDragWnd : null,
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
    
    _this.hDragWnd = document.createElement('div');
    document.body.appendChild(_this.hDragWnd);
    _this.hDragWnd.style.cssText = 'position:absolute;display:none;z-index: 1000000; background:#474747;cursor:default;';
    _this.hDragWnd.className = 'alpha_5';
  },

  attach_object : function(obj_or_id, is_dragable) {
    var obj = Q.$(obj_or_id);
    obj.setAttribute('q-drag-object', true);
    //obj.is_dragable = is_dragable || function() { return true; }
  },

  deattach_object : function(obj_or_id) {
    var obj = Q.$(obj_or_id);
    obj.removeAttribute('q-drag-object');
    //obj.is_dragable = is_dragable || function() { return true; }
  },


  is_q_drag_object : function(obj_or_id, child) {
    var obj = Q.$(obj_or_id);
    return !!obj.getAttribute('q-drag-object') 
    // return (object.is_drag() && object.id_drag(child))
  },

  is_object_dragable : function(obj, child) {
    return this.is_q_drag_object(obj);
  },

  _MouseDown : function(evt) {
    var _this = this;
    evt = evt || window.event;
    if(evt.button == Q.RBUTTON){ return; } // 屏蔽右键拖动
    var target_wnd = drag_handle = _this.nn6 ? evt.target : evt.srcElement; // 获取鼠标悬停所在的对象句柄
    
    while(target_wnd && !_this.is_q_drag_object(target_wnd) && (target_wnd != document.body)) {
      target_wnd = target_wnd.parentNode;
    }

    //if(target_wnd && (!$IsMaxWindow(target_wnd)) && $IsDragObject(target_wnd, oDragHandle)) {
    if(target_wnd && _this.is_object_dragable(target_wnd, drag_handle)) {
      var pos = Q.absPosition(target_wnd);
      _this.isdrag = true; 
      _this.hCaptureWnd = target_wnd; 
      _this.beginY = pos.top; //parseInt(_this.hCaptureWnd.style.top+0); 
      _this.y = _this.nn6 ? evt.clientY : evt.clientY; 
      _this.beginX = pos.left; //parseInt(_this.hCaptureWnd.style.left+0); 
      _this.x = _this.nn6 ? evt.clientX : evt.clientX;
      Q.printf("start x:"+_this.x+", y:"+_this.y)      
        
      //_this.hDragWnd.style.display = 'none';
      //_this.hDragWnd.style.width = _this.hCaptureWnd.offsetWidth + 'px';
      //_this.hDragWnd.style.height = _this.hCaptureWnd.offsetHeight + 'px';
      //_this.hDragWnd.style.top = pos.top + 'px'; //_this.hCaptureWnd.style.top;
      //_this.hDragWnd.style.left = pos.left + 'px'; //_this.hCaptureWnd.style.left;
        
      // 添加MouseMove事件
      _this.tmr = setTimeout(function() { Q.addEvent(document, 'mousemove', _this.MouseMove_Handler) }, 100);
      return false; 
    }
  },
    
  _MouseMove : function(evt){
    var _this = this;
    _this.isMoved = true;
    evt = evt || window.event
    //if (_this.isdrag && !$IsMaxWindow(_this.hCaptureWnd)) {
    if (_this.isdrag) {
      //_this.hDragWnd.style.display = '';
      var x = (_this.nn6?(_this.beginX+evt.clientX-_this.x):(_this.beginX+evt.clientX-_this.x));
      var y = (_this.nn6?(_this.beginY+evt.clientY-_this.y):(_this.beginY+evt.clientY-_this.y));
      //if(x < 0) {  x = 0; }

      //if(x+_this.hDragWnd.offsetWidth >  document.body.scrollWidth) {
      //  x = document.body.scrollWidth - _this.hDragWnd.offsetWidth;
      //}

      //if(y <0) {y = 0;}
      
      //if(y+_this.hDragWnd.offsetHeight >  document.body.scrollHeight) {
      //  y = document.body.scrollHeight - _this.hDragWnd.offsetHeight;
      //}
      Q.printf("moving x:"+x+", y:"+y)      
      // 移动拖动窗口位置
      _this.hCaptureWnd.style.left = x+'px';
      _this.hCaptureWnd.style.top = y+'px';
      //_this.hDragWnd.style.left = x+'px';
      //_this.hDragWnd.style.top = y+'px';
      
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
      Q.printf("end x:"+pos.left+", y:"+pos.top)      
      Q.removeEvent(document,'mousemove',_this.MouseMove_Handler);
      _this.isdrag=false;
      //_this.hDragWnd.style.display = 'none';
      var x = _this.endX-pos.left;
      var y = _this.endY-pos.top;
      Q.printf("end x:"+x+", y:"+y)      
      Q.removeEvent(document,'mousemove',_this.MouseMove_Handler);
      _this.isMoved && _this._move(_this.hCaptureWnd, x, y);
    }
    _this.isMoved=false;
  },

  _move : function(obj, pos_left, pos_top) {
    var width = obj.offsetWidth;
    var height= obj.offsetHeight;
    obj.style.left = pos_left + 'px'; 
    obj.style.top  = pos_top + 'px'; 
    obj.style.right = (pos_left+width) + 'px';
    obj.style.bottom= (pos_top+height) + 'px';
  }
});

Q.Ready(function() {
  Q.drag = new Q.draging();
}, true);

