<?php
/*--------------------------------------------------------------------
 $ module: CLASS_DATABASE For CMS@Home System
 $ date:   2010-05-07 21:23:28
 $ author: LovelyLife
 $ last modified: 2010-05-07 23:23:28
 $ copyright www.onlyaa.com
----------------------------------------------------------------------*/

(!defined('DB_MYSQL')) && define('DB_MYSQL', 1);
(!defined('DB_MSSQL')) && define('DB_MSSQL', 2);


/*
// 数据库类必须要实现的接口
interface db_interfaces {
	// 链接数据库
	function connect($host, $user, $pwd, $extra = array());
	function execute($sql, &$context);
}

*/

// 数据库基类
class CLASS_DB_BASE {
	var $linker;	// 数据库连接句柄
	function __construct() {
		$this->linker = null;
	}
 	
 	// 构造函数
	function CLASS_DB_BASE(){
		$this->__construct();
	}
	
	function connect3() {
		print('connect database!');
	}
}


$_mysql_cache = array(); // 记录集缓存

/*
name: _mapQueue  
map item struct just like

key => array(
			dicname => .., 
			func => 'callback(params)'
		)
...
...

*/
global $_mapQueue;	// cache to map queue
$_mapQueue = array();

function __db_push_map($key, $dicname, $callback) {
	global $_mapQueue;
	$_mapQueue[$key] = array(
		"dicname" => $dicname,
		"func" => $callback
	);
}

function __db_clear_map() {
	global $_mapQueue;
	unset($_mapQueue);
	$_mapQueue = array();
}

// handler to map 
function __db_handler_map(&$record) {

	global $_mapQueue;
	//print_r($_mapQueue);
	foreach($_mapQueue as $key => $vm) {
		
		$keyvalue = $record[$key];
		if(array_key_exists("dicname", $vm)) {
			$isDic = dicQuery($vm['dicname'], $dic);
			if($isDic) {
				$record[$key] = $dic[$keyvalue];
			} else {
				if(isdebug()) {
					//echo 'dictionary['.$vm['dicname'].'] not found!';
				}
			}
		}
		/*
		函数功能
		*/
		if(array_key_exists("func", $vm) && $vm["func"] != "") {
			$func = $vm['func'];
			$func = str_ireplace("@this", "'".$keyvalue."'", $func);
			$func = "\$record[\$key] = ".$func.";";
			//print($func."<br>");
			eval($func);
		}	
	}
}

// mysql
class CLASS_DB_MYSQL extends CLASS_DB_BASE {
	
	private $db_host;	// mysql服务器地址，一般为localhost
	private $db_user;	// 数据库用户名
	private $db_pswd;	// 数据库密码
	private $db_name;	// 连接的数据库
    private $db_lang;
    private $db_prefix;
	private $err;
	
	function __construct($dbhost, $dbuser, $dbpswd, $dbname, $dbprefix, $lang) {
		parent::__construct();
		$this->db_host = $dbhost;
		$this->db_user = $dbuser;
		$this->db_pswd = $dbpswd;
		$this->db_name = $dbname;
        $this->db_prefix = $dbprefix;
        $this->db_lang = $lang;
		$this->connect($this->db_host,$this->db_user,$this->db_pswd);
	}
 	
 	// 构造函数
	function CLASS_DB_MYSQL(){
		$this->__construct();
	}
	
	function connect($host, $user, $pwd, $extra = array()) {
		$pconnect = true;
		if(!$pconnect) {
			$this->linker  = mysql_connect($host, $user, $pwd);
		} else { 
			$this->linker = mysql_connect($host, $user, $pwd); 
		}
		
		//处理错误，成功连接则选择数据库
		if(!$this->linker){
			trigger_error("Can\"t use ".$db_name." : " . mysql_error(), E_USER_ERROR);
			exit();
		}
		// select database
		@mysql_select_db($this->db_name);
		// set language
		@mysql_query("SET NAMES '".$this->db_lang."';",$this->linker);
		// sql mode
		@mysql_query("SET sql_mode='' ;", $this->linker);
	}

	// 组合insert语句(insert into ##__$table(...) values(...);
	static function insertSQL($table, $fields, $isPrefix = true) {
		if(!is_array($fields)) { return -1;	}
		$setFields = array();
		$setValues = array();
		foreach($fields as $name => $value) {
			array_push($setFields, $name);
			array_push($setValues, addslashes($value));
		}
		$setFields = "`".implode("`,`", $setFields)."`";
		$setValues = "'".implode("','", $setValues)."'";
		
		$prefix = $isPrefix ? "##__" : "";
		
		$sql = "INSERT INTO `{$prefix}".$table."` (".$setFields.") VALUES(".$setValues.")";
		return $sql;	
	}

	// 组合update set部分的语句
	static function updateSQL($table, $fields, $isPrefix = true) {
		if(!is_array($fields)) { return -1;	}
		$setUpdates = array();
		foreach($fields as $name => $value) {
			array_push($setUpdates, "`".$name."` = '".$value."'");
		}
		$prefix = $isPrefix ? "##__" : "";
		$sql = "UPDATE `{$prefix}".$table."` set " .implode(",", $setUpdates);
		return $sql;
	}
	
	function setQuery($sql) {
		$this->sql = ereg_replace("##__", $this->db_prefix, $sql);
	}
	
	/*
	function doQuery($sql, $isDoMap = false) {
		//print($charset);
		$this->result = $this->Execute($sql);
		if(!$this->result) {	die("Invalid query: " . mysql_error());	}
		$records = array();
		
		if($isDoMap ) {
			while($record = $this->fetch_assoc($this->result)) {
				__db_handler_map($record);	// 映射字典表毁掉函数处理
				array_push($records, $record);	
			}
			
		} else {
			while($record = $this->fetch_assoc($this->result)) {
				array_push($records, $record);
			}
		}
		//$this->free_result($this->result);
		return $records;
	}
	*/

	function get_row($sql) {
	  $records = array();
	  if(!$this->get_results($sql, $records)) 
	    trigger_error($this->get_error());
      
	  if(empty($records)) 
	    return array();
	  else 
        return $records[0];
	}

  function dummy(&$record) {}
    
	function get_results($sql, &$result, $call=array()) {
		if(!is_array($result)) {
			$this->set_error('invalid parameter $$result');
			return false;
		}

		$this->result = $this->execute($sql);
		if(!$this->result) { 
			trigger_error($this->get_error(), E_USER_ERROR);
			return false;
		}
    
    $context = $call['context'];
    $method  = $call['method'];
    if(!is_object($context)) {
      $context = $this;
    }

    
    if(!method_exists($context, $method)) {
      $context = $this;
      $method  = 'dummy';
    }
     
    // check callback
		while($record = $this->fetch_assoc($this->result)) {
      $context->$method($record);
      array_push($result, $record);
		}

		return true;
	}
	
	// 执行无记录集返回的sql语句
	function execute_tpl($sql, &$context, &$tpl, &$out_buffer) {
		$this->result = $this->execute($sql);
		if(!$this->result) 	
			return false;

		// check $context
		if(!is_object($context) || !method_exists($context, 'item_process')) {
			$this->set_error(
			   'invalid parameter $$context or method "item_process" is not exists', true);
			return false;
		}

		while($record = $this->fetch_assoc($this->result)) {
			$out_buffer .= $context->item_process($record, $tpl);
		}

		return true;
	}
	
	function execute($sql) {
		$this->setQuery($sql);
		return mysql_query($this->sql, $this->linker);
	}

	function query_count($sql) {
	    $reg = "/^select\s+(.+?)\s+from/i";
	    $sql = preg_replace($reg, "select count(*) as qcount from", $sql, 1);
	    // print("sql: ".$sql);
	    $rs = $this->get_row($sql);
	    if(!empty($rs)) {
	      return $rs['qcount'];
            }

          return 0;
	}
	
	function num_rows() {
		if(!$this->result) {
			return 0;	
		} else {
			return mysql_num_rows($this->result);
		}
	}
	
	function get_fieldsname($table) {
		return $this->doQuery("show columns from {$table}");
	}
	
	function fetch_object($result) {
		return mysql_fetch_object($result);
	}
	
	function fetch_array($result) {
		return mysql_fetch_array($result);
	}
	
	function fetch_assoc($result) {
		return mysql_fetch_assoc($result);	
	}
	
	function get_insert_id() {
		return mysql_insert_id($this->linker);
	}
	
  function affected_rows() {
    return mysql_affected_rows($this->linker);
  }

	function free_result($result) {
		mysql_free_result($result);
	}

	
	function sql_push($cachename, $sql) {
		global $_mysql_cache;
		$_mysql_cache[$cachename] = $sql;
	}
	
	function sql_query($cachename) {
		global $_mysql_cache;
		return $_mysql_cache[$cachename];	
	}
	
	function close() {
		if($this->linker) {
			@mysql_close($this->linker);
		}
	}
	
	private function set_error($msg, $interrupt = false) {
		$this->err = $msg;
		if($interrupt) {
			trigger_error($this->err, E_USER_ERROR);
		}
	}

	function get_error() {
		return $this->err."\r\n".mysql_error();
	}
}

/*

// mssql
class CLASS_DB_MSSQL extends CLASS_DB_BASE implements db_interfaces {
	
	var $db_host;	// mysql服务器地址，一般为localhost
	var $db_user;	// 数据库用户名
	var $db_pswd;	// 数据库密码
	var $db_name;	// 连接的数据库
	
	function __construct() {
		parent::__construct();
		$this->db_host = $GLOBALS['APP_CFG']["DB_HOST"];
		$this->db_user = $GLOBALS['APP_CFG']["DB_USER"];
		$this->db_pswd = $GLOBALS['APP_CFG']["DB_PSWD"];
		$this->db_name = $GLOBALS['APP_CFG']["DB_NAME"];
		$this->connect($this->db_host,$this->db_user,$this->db_pswd);
	}
 	
 	// 构造函数
	function CLASS_DB_MSSQL(){
		$this->__construct();
	}
	
	function connect($host, $user, $pwd, $extra = array()) {

		$this->linker  = mssql_connect($host, $user, $pwd);
		
		//处理错误，成功连接则选择数据库
		if(!$this->linker){
			die ("Can\"t use ".$db_name." : " . mysql_error());
			exit();
		}
		// select database
		@mssql_select_db($this->db_name);
		// set language
		@mssql_query("SET NAMES '".$GLOBALS['APP_CFG']["DB_LANGUAGE"]."';",$this->linker);
		// sql mode
		@mssql_query("SET sql_mode='' ;", $this->linker);
	}
	
	function setQuery($sql) {
		global $CH_CFG, $cmspath, $site;
		$this->sql = ereg_replace("##__", $GLOBALS['APP_CFG']["DB_TAG"], $sql);
	}
	
	function doQuery($sql, $isDoMap = false) {
		//print($charset);
		$this->result = $this->Execute($sql);
		if(!$this->result) {	die("Invalid query: ");	}
		$records = array();
		if($isDoMap ) {
			while($record = $this->fetch_assoc($this->result)) {
				__mysql_map_handler($record);	// 映射字典表毁掉函数处理
				array_push($records, $record);	
			}
		} else {
			while($record = $this->fetch_assoc($this->result)) {
				array_push($records, $record);
			}
		}
		//$this->free_result($this->result);
		return $records;
	}
	
	// 执行无记录集返回的sql语句
	function doExecute($sql) {
		$this->result = $this->Execute($sql);
		if(!$this->result) {	die("Invalid query: " . mysql_error());	}
	}
	
	function Execute($sql) {
		$this->setQuery($sql);
		return mssql_query($this->sql, $this->linker);
	}
	
	function num_rows() {
		if(!$this->result) {
			return 0;	
		} else {
			return mssql_num_rows($this->result);
		}
	}
	
	function getRow($sql) {
		$rs = $this->doQuery($sql);
		if(count($rs) > 0) {
			return $rs[0];
		} else {
			return null;
		}
	}
	
	function get_fieldsname($table) {
		return $this->doQuery("show columns from {$table}");
	}
	
	function fetch_object($result) {
		return mssql_fetch_object($result);
	}
	
	function fetch_array($result) {
		return mssql_fetch_array($result);
	}
	
	function fetch_assoc($result) {
		return mssql_fetch_assoc($result);	
	}
	
	function get_insert_id() {
		return mssql_insert_id($this->linker);
	}
	
	function free_result($result) {
		mssql_free_result($result);
	}
	
	static function sql_push($cachename, $sql) {
		global $_mysql_cache;
		$_mysql_cache[$cachename] = $sql;
	}
	
	static function sql_query($cachename) {
		global $_mysql_cache;
		return $_mysql_cache[$cachename];	
	}
	
	function close() {
		if($this->linker) {
			@mysql_close($this->linker);
		}
	}
	
	function error($errmsg) {
		$errmsg = "";
		$errmsg.= "";
		return $errmsg;
	}
}
*/

// 数据库类工厂, 默认为mysql数据库
function createdb($dbtype, $dbparams) {
	switch($dbtype) {
	case 'mssql':
		$classname = 'CLASS_DB_MSSQL';
		break;
	default:
		$classname = 'CLASS_DB_MYSQL';
	}
	
	// 如果类没有定义则
	if(!class_exists($classname)) {
		die('create db['.$classname.'] error!');
		return null;
	}
	return(new $classname($dbparams['host'], 
        $dbparams['user'], 
        $dbparams['pswd'], 
        $dbparams['name'], 
        $dbparams['prefix'],
        $dbparams['lang']
    ));
}



/* for example:
$db = createdb();
$db->connect();
*/
?>
