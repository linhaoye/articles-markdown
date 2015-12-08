<?php 
$arr = array(
	array('id' => 1, 'pid' => 9, 'name' => 'Languages'),
	array('id' => 2, 'pid' => 1, 'name' => "C"),
	array('id' => 3, 'pid' => 1, 'name' => "C++"),
	array('id' => 4, 'pid' => 1, 'name' => "C#"),
	array('id' => 5, 'pid' => 1, 'name' => "Python"),
	array('id' => 6, 'pid' => 1, 'name' => "PHP"),
	array('id' => 7, 'pid' => 1, 'name' => "Java"),
	// array('id' => 8, 'pid' => 0, 'name' => "tools"),
	array('id' => 9, 'pid' => 8, 'name' => "debug tool"),
	array('id' => 10, 'pid' => 8, 'name' => "text tool"),
	array('id' => 11, 'pid' => 9, 'name' => "fire bug"),
	array('id' => 12, 'pid' => 9, 'name' => "javascript console"),
);

function array_to_tree(array $array,
						$root   = 0,
						$level  = 0,
						$id     = 'id',
						$pid    = 'pid',
						$title  = "title",
						$anchor = "|-"
					)
{
	static $format_tree = array();

	foreach ($array as $value) {
		if ($value[$pid] == $root) {
			$value['level'] = $level;
			$value[$title]  = str_repeat('  ', $level * 2) . $anchor;
			$format_tree[]  = $value;
			array_to_tree($array, $value[$id], $level + 1);
		}
	}

	return $format_tree;
}

function array_to_tree2(array $array,
						$id  = 'id',
						$pid = 'pid',
						$son = 'child'
					)
{
	$format_tree = array();	// 格式化数据
	$array_map = array();	// 临时扁平数据

	foreach ($array as $value) {
		$array_map[$value[$id]] = $value;
	}

	foreach ($array as $value) {
		if (isset($array_map[$value[$pid]]) && $value[$id] != $value[$pid]) {
			if (!isset($array_map[$value[$pid]] [$son])) {
				$array_map [$value[$pid]] [$son] = array();
			}
			$array_map[$value[$pid]] [$son] [] = & $array_map[$value[$id]];
		} else {
			$format_tree[] = & $array_map[$value[$id]];
		}
	}

	return $format_tree;
}

var_dump(array_to_tree($arr, 9));
var_dump(array_to_tree2($arr));
 ?>