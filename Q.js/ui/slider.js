/*-------------------------------------------------------
  UI library
  function: select 美化
  date: 2008-06-12
  author: lovelylife
  the component can be used as UI.components[id]
---------------------------------------------------------*/

function find_item(root, cmp) {
  if(!root) 
    return null;

  var items = root.childNodes;
  for(var i=0; i < items.length; i++) {
    var item = items[i];
    if(cmp(item)) {
      return item;
    } else {
      var e = find_item(item, cmp);
      if(e)
        return e;
    }
  }

  return null;
}

Q.slider = Q.extend({
  min : 0,
  max : 100,
  duration  : 1,
  hwnd  : null,
  thumb : null,
  construct: function(config) {
    var _this = this;
    config = config || {};
    this.type = (config.type == 'vt')?'vt':'hr';
    this.min = config.min;
    this.max = config.max;
    this.duration = config.duration || 1;
    if(typeof config.on_xscroll == 'function') {
      this.on_xscroll = config.on_xscroll;
    }
    if(typeof config.on_yscroll == 'function') {
      this.on_yscroll = config.on_yscroll;
    }
    this.duration = config.duration || 1;
    this.duration = config.duration || 1;
    this.hwnd = Q.$(config.id);
    this.thumb = find_item(this.hwnd, function(e) { return (e && e.nodeType == Q.ELEMENT_NODE) && (e.className.indexOf('q-thumb') >= 0); });
      
    if(this.thumb) {
      Q.drag.attach_object(this.thumb, {self: true, onmove: function(x, y) {
        if(_this.type != 'hr') {
          if(y < 0) 
            y = 0;
          if(y > (_this.hwnd.offsetHeight-_this.thumb.offsetHeight))
            y =  (_this.hwnd.offsetHeight-_this.thumb.offsetHeight);
          _this.on_yscroll(Math.floor(_this.min + ((_this.max-_this.min)*(y*1.0)/(_this.hwnd.offsetHeight-_this.thumb.offsetHeight))));
          _this.thumb.style.top = y + 'px';
        } else {          
          if(x < 0) 
            x = 0;
          if(x > (_this.hwnd.offsetWidth-_this.thumb.offsetWidth))
            x =  (_this.hwnd.offsetWidth-_this.thumb.offsetWidth);
          _this.on_xscroll(Math.floor(_this.min + ((_this.max-_this.min)*(x*1.0)/(_this.hwnd.offsetWidth-_this.thumb.offsetWidth))));
          _this.thumb.style.left = x + 'px';
        }
      }});
    }
  },

  on_xscroll : function(value) {}, 
  on_yscroll : function(value) {},
}); 

