<?php 

function guid()
{
    $charid = strtoupper(md5(uniqid(mt_rand(), true)));
    $uuid = substr($charid, 0, 8).
        substr($charid, 8, 4).
        substr($charid,12, 4).
        substr($charid,16, 4).
        substr($charid,20,12);
    return $uuid;	
}

/**
 * 生成一个token
 * 
 * @param  string $name 令牌名称
 * @return string
 */
function genToken($name = "hash")
{
	session_start();
	$key = guid();
	$value = guid();
	$_SESSION[$name][$key] = $value;

	return $key . '_' . $value;
}

/**
 * 验证令牌
 * 
 * @param  string $name 令牌名称
 * @return boolean
 */
function chkToken($name = "hash")
{
	session_start();

	if (!isset($_REQUEST[$name]) || !isset($_SESSION[$name])) { //页面没有传token的参数
		return false;
	}

	list($key, $value) = explode('_', $_REQUEST[$name]);

	//值相同认证通过
	if (isset($_SESSION[$name][$key]) && $value && $_SESSION[$name][$key] === $value ) {
		unset($_SESSION[$name][$key]);
		return true;
	}

	return false;
}

# 测试
 ?>