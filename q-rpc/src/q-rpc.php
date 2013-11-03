<?php

echo "hello php rpcclient!\n";


class QRPC_MESSAGE {
  public $type;
  public $channel;
  public $body;
  const message_request = 0;
  const message_response= 1; 
  function __construct() {}
  function QRPC_MESSAGE() {  $this->__construct(); }

  // encode to binary string
  function to_binary() {
    return pack('lhlha*h', 
      $this->type, 0xFE, 
      $this->channel, 0xFE,
      $this->body, 0xFE);
  }

  function from_binary($bstr) {
    var_dump($bstr);
    echo "from_binary:".$bstr." end\n";
    $arr = unpack('ltype/h/lchannel/h/a*body', $bstr);
    if($arr == false) {
    //  echo "unpack error\n";
    } else { 
      print_r($arr);
      $this->type = $arr['type'];
      $this->channel = $arr['channel'];
      $this->body = $arr['body'];
    }
  }
};

// test code
$struct = new QRPC_MESSAGE;
$struct->type = 1;
$struct->channel = 2;
$struct->body = "test body ok!";

$struct2 = new QRPC_MESSAGE;
$struct2->from_binary($struct->to_binary());
print_r($struct);
print_r($struct2);



class QRPC_SERIALIZER {
  protected $body_;
  function __construct() {
    $this->body_ = array();
  }
 
  function QRPC_SERIALIZER() {
    $this->__construct();
  }

  function to_string() {
    return json_encode($this->body_);
  }
 
  function from_string($json_str) {
    $this->body_ = json_decode($json_str, true);
  }

};

class QRPC_REQUEST extends QRPC_SERIALIZER {
  function __construct() {
    parent::__construct();
  }
  function QRPC_REQUEST() {$this->__construct(); }
  function action() { return $this->body_['action']; }
  function set_action($action) {  $this->body_['action'] = $action; }
  function& params() { return $this->body_['parameters']; }
  function set_params($params) {
    $this->body_['parameters'] = $params;
  }

};

class QRPC_RESPONSE extends QRPC_SERIALIZER {
  function __construct() {
    parent::__construct();
  }
  function QRPC_RESPONSE() {$this->__construct(); }
  function get_result_code() { return $this->body_['result_code']; }
  function put_result_coden($c) {  $this->body_['result_code'] = $c; }

};


class QRPC {
  private $server_;
  private $port_;
  private $services_proxy_;  
  private $sock_;
  private $connection_;
  function __construct($server, $port) {
    $this->server_ = $server;
    $this->port_ = $port; 
    $this->sock_ = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
    $this->connection_ = socket_connect($this->sock_, $server, $port);
    if(!$this->connection_) {
      socket_close($this->sock_);
    }

    $pkg = $this->read_pkg();
    $this->_init_interfaces($pkg);
     
  }

  function QRPC($server, $port) {
    $this->__construct($server, $port);
  }

  function _init_interfaces($interfaces) {
    $is = json_decode($interfaces, true);
    $services = $is['services'];
    $services_size = count($services);
    for($i=0; $i < $services_size; $i++) {
      $service_object = &$services[$i];
      $service_functions = &$service_object['funcs'];  
      $service_functions_size = count($service_functions);
      $import_functions = array();
      for($j = 0; $j < $service_functions_size; $j++) {
	$function_object = $service_functions[$j];
        $import_functions[$function_object['name']] = $function_object['id'];
      }
      $this->interfaces_[$service_object['name']] = array(
        'id' => $service_object['id'],
	'funcs' => $import_functions
      );
    }
  }

  function get_service($service_name) {
    if(array_key_exists($service_name, $this->interfaces_)) {
      $service_object = $this->interfaces_[$service_name];
      if(array_key_exists('id', $service_object)) {
        return $service_object['id'];
      }
    }

    return -1;
  }

  function get_func($service_name, $method_name) {
    if(array_key_exists($service_name, $this->interfaces_)) {
      $service_object = $this->interfaces_[$service_name];
      if(array_key_exists('funcs', $service_object)) {
        $functions = $service_object['funcs'];
        if(array_key_exists($method_name, $functions))
	  return $functions[$method_name];
      }
    }
  
    return -1;
  }

  function read_pkg() {
    $d = socket_read($this->sock_, 4, PHP_BINARY_READ);
    $size = unpack('l', $d);
    echo "read header size: " . $size[1] . "\n";
    $data = socket_read($this->sock_, $size[1]);
    echo $data;
    return $data;
  }

  function write_pkg($data) {
    $length = strlen($data);
    $header = pack('l', $length);
    socket_write($this->sock_, $header, 4);  
    socket_write($this->sock_, $data, $length);
  }
  function call($service_name, $method_name, $params) {
    $channel_id = $this->get_service($service_name);   
    $func_id = $this->get_func($service_name, $method_name);    
    echo $func_id;
    if($func_id < 0) {
      trigger_error('rpc service('.$service_name.') or method('.$method_name.') is not exists', E_USER_ERROR);
    }

    $msg_response = new QRPC_MESSAGE();
    $msg_request  = new QRPC_MESSAGE();
    $msg_request->type = QRPC_MESSAGE::message_request; 
    $msg_request->channel = $channel_id;

    $request = new QRPC_REQUEST;
    $request->set_action($func_id);
    $request->set_params($params);
    $msg_request->body = $request->to_string();
    $this->write_pkg($msg_request->to_binary());
    $data = $this->read_pkg();
    $msg_response->from_binary($data);
    $response = new QRPC_RESPONSE;
    $response->from_string($msg_response->body);
    print_r($response);
    return $response;
  }

  function __toString() {
    $str = "server: " . $this->server_ . "\n";
    $str .="service: " . $this->service_ . "\n";
    $str .="interfaces: " . json_encode($this->interfaces_) . "\n";
    return $str;
  }


}
$rpc_client = new QRPC("wayixia.com", 5555);

echo $rpc_client;
$rpc_client->call('test_service', 'add', array());
//$rpc_client->call('test_service', 'test', array());



?>
