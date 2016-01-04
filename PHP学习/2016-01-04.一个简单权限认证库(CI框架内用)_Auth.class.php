<?php if ( ! defined('IN_SX')) exit('No direct script access allowed');
/**
 * 涉及到的表:
 *
 * 用户组
 * CREATE TABLE `group` (
			`group_id`  mediumint(8) UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '组id' ,
			`title`  char(100) CHARACTER SET utf8 COLLATE utf8_general_ci NOT NULL DEFAULT '' COMMENT '组名' ,
			`status`  tinyint(1) NOT NULL DEFAULT 1 COMMENT '状态' ,
			`ind_code`  varchar(80) CHARACTER SET utf8 COLLATE utf8_general_ci NOT NULL DEFAULT '0' COMMENT '标识码' ,
			`rules`  text CHARACTER SET utf8 COLLATE utf8_general_ci NOT NULL COMMENT '规则名' ,
			PRIMARY KEY (`group_id`)
		)
		ENGINE=InnoDB
		DEFAULT CHARACTER SET=utf8 COLLATE=utf8_general_ci
		COMMENT='用户组'
 *
 * 菜单(权限)
 * CREATE TABLE `menu` (
			`id`  int(10) UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '文档ID' ,
			`title`  varchar(50) CHARACTER SET utf8 COLLATE utf8_general_ci NOT NULL DEFAULT '' COMMENT '标题' ,
			`pid`  int(10) UNSIGNED NOT NULL DEFAULT 0 COMMENT '上级分类ID' ,
			`sort`  int(10) UNSIGNED NOT NULL DEFAULT 0 COMMENT '排序（同级有效）' ,
			`url`  char(255) CHARACTER SET utf8 COLLATE utf8_general_ci NOT NULL DEFAULT '' COMMENT '链接地址' ,
			`root_node`  int(10) NOT NULL COMMENT '根节点' ,
			`hide`  tinyint(1) UNSIGNED NOT NULL DEFAULT 0 COMMENT '是否隐藏' ,
			`tip`  varchar(255) CHARACTER SET utf8 COLLATE utf8_general_ci NOT NULL DEFAULT '' COMMENT '提示' ,
			`group`  varchar(50) CHARACTER SET utf8 COLLATE utf8_general_ci NULL DEFAULT '' COMMENT '分组' ,
			`status`  tinyint(1) NOT NULL DEFAULT 1 ,
			`is_dev`  tinyint(1) UNSIGNED NOT NULL DEFAULT 0 COMMENT '是否仅开发者模式可见' ,
			PRIMARY KEY (`id`),
			INDEX `pid` (`pid`) USING BTREE 
		)
		ENGINE=InnoDB
		DEFAULT CHARACTER SET=utf8 COLLATE=utf8_general_ci
		COMMENT='菜单'
 *
 * 白名单(菜单|权限)
 * CREATE TABLE `whites` (
			`id`  int(10) NOT NULL AUTO_INCREMENT ,
			`title`  varchar(80) CHARACTER SET utf8 COLLATE utf8_general_ci NOT NULL COMMENT '规则名' ,
			`url`  varchar(255) CHARACTER SET utf8 COLLATE utf8_general_ci NOT NULL COMMENT 'url' ,
			`status`  tinyint(2) NOT NULL DEFAULT 1 COMMENT '状态' ,
			PRIMARY KEY (`id`)
		)
		ENGINE=InnoDB
		DEFAULT CHARACTER SET=utf8 COLLATE=utf8_general_ci
		COMMENT='权限白名单'
 *
 * 授权表
 * CREATE TABLE `group_access` (
			`uid`  mediumint(8) UNSIGNED NOT NULL ,
			`group_id`  mediumint(8) UNSIGNED NOT NULL ,
			PRIMARY KEY (`uid`, `group_id`),
			UNIQUE INDEX `uid_group_id` (`uid`, `group_id`) USING BTREE ,
			INDEX `uid` (`uid`) USING BTREE ,
			INDEX `group_id` (`group_id`) USING BTREE 
		)
		ENGINE=InnoDB
		DEFAULT CHARACTER SET=utf8 COLLATE=utf8_general_ci
		COMMENT='授权表'
 *
 * 以下权限系统代码适用于一个用户只属于一个用户组的对应关系, 若要多对多, 请修改一下代码的实现
 */
class Auth
{
	/**
	 * ci超级句柄类
	 * 
	 * @var object
	 */
	private $ci = NULL;	

	/**
	 * 认证开关
	 * 
	 * @var boolean
	 */
	private $authon = true;

	/**
	 * 认证模式
	 * 
	 * @var integer
	 */
	private $auth_type = 3;

	/**
	 * 表前缀
	 * 
	 * @var string
	 */
	private $tb_prefix = "tb_";

	/**
	 * 旧认证系统权限集, 兼容以前代码
	 * 
	 * @var array
	 */
	public $rights = array();

	/**
	 * 新版规则集
	 * 
	 * @var array
	 */
	private $access_rules = array();

	/**
	 * 超管id
	 * 
	 * @var integer
	 */
	private $root = 1;

	/**
	 * 权根认证表
	 * 
	 * @var array
	 */
	private $tables = array(
		'auth_group'        => 'auth_group',
		'auth_group_access' => 'auth_group_access',
		'auth_rule'         => 'menu',
		'auth_user'         => 'admins',
	);

	/**
	 * 构造函数
	 * 
	 */
	public function __construct()
	{
		$this->ci = & get_instance();

		$this->_init();
		$this->_validation();
	}

	/**
	 * 初始化
	 * 
	 * @return void
	 * 
	 */
	public function _init()
	{
		//顶部菜单
		$where['pid']  = 0;
		$where['hide'] = 0;

		$this->top_menus = $this->ci->db->from("menu")
								->where($where)
								->get()
								->result();

		$this->controller = $this->ci->uri->rsegment(1);
		$this->method     = $this->ci->uri->rsegment(2);

		//当前菜单
		$this->current = $this->ci->db->from("menu")
								->select("id,pid,title,root_node")
								->where("url", $this->controller ."/". $this->method)
								->get()
								->row();

		//当前登录用户所属组规则集,超管除外
		if ($this->ci->_admin->uid != $this->root)
		{
			$this->access_rules = $this->get_access_rules($this->ci->_admin->uid, 1);
		}

		//兼容旧权限系统(处理废弃代码)
		$this->compatible_old_permsystem();
	}

	/**
	 * 
	 * 拦截器
	 * 
	 * @return void
	 * 
	 */
	public function _validation()
	{
		//超管不作检查
		if ($this->ci->_admin->uid == $this->root)
		{
			return;
		}

		$uri = $this->controller .'/'. $this->method;

		//不在白名单
		if (!$this->filter_white_rules($uri))
		{
			//认证失败
			if (!$this->check($uri))
			{
				//重定向到用户入口层或都提示	
				$this->_message("你没有访问的权限!", site_url($this->access_rules[0]));
			}
		}
	}

	/**
	 * 信息提示
	 * 
	 * @param  string  $msg 	提示信息
	 * @param  string  $goto 	跳转url
	 * @param  boolean $auto 	是否自跳转
	 * @param  string  $fix 	带外参数
	 * @param  integer $pause 	跳转所需时间
	 * @return void
	 * 
	 */
	public function _message($msg, $goto = '', $auto = true, $fix = '', $pause = 2000)
	{
		if ($goto == '')
			$goto = isset ( $_SERVER ['HTTP_REFERER'] ) ? $_SERVER ['HTTP_REFERER'] : site_url ();
		else
			$goto = strpos ( $goto, 'http' ) !== false ? $goto : backend_url ( $goto );

		$goto .= $fix;

		$data = array(
			'msg'   => $msg,
			'goto'  => $goto,
			'auto'  => $auto,
			'pause' => $pause 
		);

		//如果是ajax请求
		if ($this->ci->input->is_ajax_request())
		{
			$data['error'] = 9999;
			echo json_encode($data);exit;
		}

		$data["auth"] = $this;

		$this->ci->load->view('sys_auth_message', $data);

		echo $this->ci->output->get_output();
		exit;
	}

	/**
	 * 粗略权限检查
	 * 	
	 * @param  string|array  $rule_name 规则集
	 * @param  int  $uid       用户id
	 * @param  integer $type      认证类型
	 * @return boolean
	 *
	 */
	public function check($rule_name, $uid = "", $type = 1,$relation = 'or')
	{
		//认证若关闭
		if (!$this->authon)
		{
			return true;
		}

	 	//若空则为当前登录用户
		$access_rules_list = empty($uid) ? $this->access_rules: $this->get_access_rules($uid, $type);

		if (is_string($rule_name))
		{
			$rule_name = strtolower($rule_name);
			//含逗号分隔的规则表
			if (strpos($rule_name, ',') !== false)
			{
				$rule_name = explode(',', $rule_name);
			}
			else
			{
				//只有一个情况下
				$rule_name = array($rule_name);
			}
		}

		//保存验证通过的规则名
		$save_access_rules_list = array();

		foreach ($access_rules_list as $rule)
		{
			if (in_array($rule, $rule_name))
			{
				$save_access_rules_list[] = $rule;
			}
		}

		//or
		if ($relation == 'or' and !empty($save_access_rules_list))
		{
			return true;
		}

		//and
		$diff = array_diff($rule_name, $save_access_rules_list);
		if ($relation == 'and' and empty($diff))
		{
			return true;
		}

		return false;
	}	

	/**
	 * 严格权限检查(使用正则方式)
	 * 	
	 * @param  string|array  $rule_name 规则集
	 * @param  int  $uid       用户id
	 * @param  integer $type      认证类型
	 * @param  string  $mode      认证方式
	 * @param  string  $relation  关系 or|and
	 * @return boolean
	 *
	 */
	public function strict_check($rule_name, $uid, $type = 1, $mode = 'url', $relation = 'or')
	{
		//认证若关闭
		if (!$this->authon)
		{
			return true;
		}

		$access_rules_list = $this->get_access_rules($uid, $type);

		if (is_string($rule_name))
		{
			$rule_name = strtolower($rule_name);
			//含逗号分隔的规则表
			if (strpos($rule_name, ',') !== false)
			{
				$rule_name = explode(',', $rule_name);
			}
			else
			{
				//只有一个情况下
				$rule_name = array($rule_name);
			}
		}

		//保存验证通过的规则名
		$save_access_rules_list = array();

		if ($mode == 'url')
		{
			$request = unserialize(strtolower(serialize($_REQUEST)));
		}

		foreach ($access_rules_list as $rule)
		{
			$query = preg_replace('/^.+\?/U', '', $rule);

			if ($mode == 'url' && $query != $rule)
			{
				//解析规则的param
				parse_str($query, $param);
				$intersect = array_intersect_assoc($request, $param);
				$rule = preg_replace('/\?.*$/U', '', $rule);

				//如果节点相符且url参数满足
				if (in_array($rule, $rule_name) && $intersect == $param)
				{
					$save_access_rules_list[] = $rule;
				}
			}
			else if (in_array($rule, $rule_name))
			{
				$save_access_rules_list[] = $rule;
			}
		}

		//or
		if ($relation == 'or' and !empty($save_access_rules_list))
		{
			return true;
		}

		//and
		$diff = array_diff($rule_name, $save_access_rules_list);
		if ($relation == 'and' and empty($diff))
		{
			return true;
		}

		return false;
	}

	/**
	 * 过滤白名单
	 * 
	 * @param  string $rule_name 规则名
	 * @return void
	 * 
	 */
	public function filter_white_rules($rule_name)
	{
		$this->ci->settings->load("auth/white_rules.php");

		$whites = &setting('white_rules');

		if (isset($whites['rules']) && !empty($whites['rules']))
		{
			return in_array(strtolower($rule_name), $whites['rules']);
		}

		$whites = $this->ci->db->from("white_rules")->where(array('status'=>1))->get()->result();

		foreach ($whites as $w)
		{
			if (strtolower($w->url) == strtolower($rule_name))
			{
				return true;
			}
		}

		return false;
	}

	/**
	 * 获取规则集
	 * 
	 * @param  int $uid  用户id
	 * @param  string $type 类型
	 * @return array
	 * 
	 */
	public function get_access_rules($uid, $type)
	{
		static $access_rules_list = array();

		$t = implode(',', (array)$type);

		if (isset($access_rules_list[$uid.$t]))
		{
			return $access_rules_list[$uid.$t];
		}

		//session缓存的rules
		if ($this->auth_type == 2 && $this->session->userdata("auth_list_".$uid.$t))
		{
			return $this->ci->session->userdata("auth_list_".$uid.$t);
		}

		//文件缓存的rules(这里用于登录用户)
		if ($this->auth_type == 3)
		{
			$this->ci->settings->load("auth/role_" .$this->ci->_admin->role. ".php");

			$access_rules = & setting('group_info');

			if (isset($access_rules['rules']) && !empty($access_rules['rules']))
			{
				$access_rules_list[$uid.$t] = $access_rules['rules'];

				return $access_rules['rules'];
			}
		}

		//获取用户的所属组
		$groups = $this->get_groups($uid);

		$ruleids = array();
		foreach ($groups as $value)
		{
			$ruleids = array_merge($ruleids, explode(',', trim($value->rules, ',')));
		}

		$ruleids = array_unique($ruleids);

		//规则集空
		if (empty($ruleids))
		{
			$access_rules_list[$uid.$t] = array();
			return array();
		}

		//查询规则集
		$tb_auth_rule = $this->tb_prefix . $this->tables["auth_rule"];

		$sql = sprintf(
			"SELECT * FROM %s WHERE id IN(%s) AND status=%d", 
			$tb_auth_rule, 
			implode(',', $ruleids), 
			1
		);

		$access_rules = $this->ci->db->query($sql)->result();

		//遍历规则集, 将符合的的规则集放置在一个数组中
		$tmp = array();
		foreach ($access_rules as $rule)
		{
			if (!empty($rule->condition))
			{
				//暂时不处理带外参数
			}
			else
			{
				$tmp[] = strtolower($rule->url);
			}
		}
		$access_rules_list[$uid.$t] = $tmp;

		//写缓存, session
		if ($this->auth_type == 2)
		{
			$this->ci->session->set_userdata("auth_list_".$uid.$t, $tmp);
		}

		return array_unique($tmp);
	}	

	/**
	 * 缓存用户组规则集,权限授权为一对多关系(即一个用户只属一个用户组)
	 * 	
	 * @param  int $gid 组id
	 * @return void
	 * 
	 */
	public function cache_access_rules($gid)
	{
		$group = $this->ci->db->from($this->tables['auth_group'])->where("group_id", $gid)->get()->row();

		if ($group && $group->rules != "")
		{
			$id_arr = explode(',', $group->rules);
			$rules  = $this->ci->db->from("menu")->select("url")->where_in("id", $id_arr)->get()->result();

			$rules_array = array();

			if ($rules)
			{
				foreach ($rules as $key => $r)
					$rules_array[] = $r->url;
			}

			//创建缓存目录
			if ($this->ci->platform->get_type() == 'default')
			{
				if (! file_exists(SX_SHARE_PATH .'settings/auth'))
				{
					mkdir(SX_SHARE_PATH .'settings/auth', 0777);
				}
			}

			//缓存路径
			$cache_path = SX_SHARE_PATH .'settings/auth/role_'.$gid.'.php';

			//数据
			$cache_data = array(
				'id'       => $group->group_id,
				'title'    => $group->title,
				'ind_code' => $group->ind_code,
				'status'   => $group->status,
				'rules'    => $rules_array,
			);

			$this->ci->platform->cache_write($cache_path, array_to_cache("setting['group_info']", $cache_data));
		}
	}

	/**
	 * 主要兼容以前的旧的权限系统的代码
	 * 
	 * @return void
	 * 
	 */
	public function compatible_old_permsystem()
	{
		$this->rights = array(
			'id'              => $this->ci->_admin->uid,
			'name'            => '',
			'rights'          => $this->access_rules,
			'ind_code'        => '',
			'models'          => array(
					0 => '', 
				),
			'category_models' => array(
					0 => '', 
				),
			'plugins'         => array(
					0 => '',
				),
		);
	}

	/**
	 * 
	 * 获取用户的用户级
	 * 
	 * @param  int $uid 用户id
	 * 
	 * @return array
	 */
	public function get_groups($uid)
	{
		static $groups = array();

		if (isset($groups[$uid]))
		{
			return $groups[$uid];
		}

		$tb_auth_group_access = $this->tb_prefix.  $this->tables['auth_group_access'];
		$tb_auth_group = $this->tb_prefix. $this->tables['auth_group'];

		$sql = sprintf(
			'SELECT uid,a.group_id,title,rules FROM %s AS a INNER JOIN %s AS g ON a.group_id = g.group_id WHERE a.uid = %d AND g.status = %d',
			$tb_auth_group_access,
			$tb_auth_group,
			$uid,
			1
		);

		$user_groups = $this->ci->db->query($sql)->result();

		$groups[$uid] = $user_groups ? $user_groups: array();

		return $groups[$uid];
	}

	/**
	 * 输出左边栏菜单
	 * 		
	 * @return void
	 * 
	 */
	public function show_left_menus()
	{
		if ($this->current)
		{
			//选中顶部导航栏
			$nav = $this->ci->db->from("menu")->where("id", $this->current->root_node)->get()->row();

			if (empty($nav)) goto label;

			$second_menus = $this->ci->db->from("menu")
									->select("id,pid,title,url")
									->where("pid", $nav->id)
									->get()
									->result();

			//查找二级菜单
			foreach ($second_menus as $m)
			{
				//作粗略过滤 (不调用$this->check())
				if (!in_array($m->url, $this->access_rules) && 
					$this->ci->_admin->uid != $this->root)
					continue;

				printf("<li><span><a href='javascript:;' url='%s'>%s</a></span><ul name='menu'>",
					backend_url($m->url), $m->title);

				$third_menus = $this->ci->db->from("menu")
										->select("id,pid,title,url")
										->where("pid", $m->id)
										->get()
										->result();

				//查找三级菜单
				foreach ($third_menus as $n)
				{
					if (!in_array($n->url, $this->access_rules) && 
					$this->ci->_admin->uid != $this->root) //超管
						continue;

					//高亮选中
					$is_highlight = $this->highlight_leftmenu($n);

					$class = $is_highlight ? "class='selected'" : "";

					printf ("<li %s><a style='margin-left:40px;' href='%s'>%s</a></li>", 
						$class,
						backend_url($n->url),
						$n->title);
				}

				print "</ul></li>";
			}
		}
		label:
			return;
	}

	/**
	 * 自动折叠菜单
	 * 
	 * @return void
	 * 
	 */
	public function auto_folder_menu()
	{
		echo <<<EOF
		    (function () {
		        var $current = $("ul.submenu ul[name='menu']>li.selected");

		        if ($current.length >0)
		            $current.parent().show('fast').closest('li').siblings().find("ul[name='menu']").hide();
		        else
		            $("ul.submenu ul[name='menu']").hide('fast', function() {});

		    })();

		    $("ul.submenu").on("click", "span", function () {
		        var $this = $(this);
		        $this.next().slideToggle('fast').closest('li').siblings().find("ul[name='menu']").hide();
		    });
EOF;
	}

	/**
	 * 高亮侧边三级菜单栏
	 * 
	 * @param object $menu
	 * @return void
	 * 
	 */ 
	public function highlight_leftmenu($menu)
	{
		$current = $this->controller .'/'. $this->method;

		if (strtolower($menu->url) == strtolower($current) || $this->current->pid == $menu->id)//当current为操作时
		{
			return 1;
		}

		return 0;
	}

	/**
	 * 高亮侧边(旧系统)
	 * 
	 * @param  string $uri
	 * @return void
	 * 
	 */
	public function highlight_leftmenu_bak($uri)
	{

		$this->ci->load->library("session");

		$segments = explode('/', $uri);
		$menu     = $this->ci->session->all_userdata();

		if (isset($menu['left_menu_name']))
			return $menu['left_menu_name'] == implode('@', $segments) ? 1: 0;

		return 0;
	}

	/**
	 * 输出顶部菜单
	 * 
	 * @return void 
	 */
	public function show_top_menus()
	{
		$last_index = count($this->top_menus) - 1;

		foreach ($this->top_menus as $k => $m)
		{
			if (!in_array($m->url, $this->access_rules) && $this->ci->_admin->uid != $this->root)
				continue;

			$class = "";
			
			if ($k == 0)
				$class = "first";
			else if ($k == $last_index)
				$class = "last";

			if ($this->current)
			{
				if ($this->current->root_node == $m->id) 
					$class .= " selected";
			}

			printf("<li class='%s'><a href='%s'>%s</a></li>", 
				$class, 
				backend_url($m->url), 
				$m->title);
		}
	}

	/**
	 * 
	 * 输出当前顶部标题栏
	 * 
	 * @return void
	 */
	public function show_top_menu_name()
	{
		if ($this->current)
		{
			foreach ($this->top_menus as $key => $value)
			{
				if ($value->id == $this->current->root_node)
					echo $value->title;
			}
		}

		echo "";
	}

	/**
	 * 单一隐藏控制方法
	 * 
	 * @return int
	 * 
	 */
	public function noc_hidden_page()
	{
		$is_hidden = handle_query($this->ci->input->get(HIDDEN_PAGE_NAME, true));

		if ($is_hidden)
			return $is_hidden == HIDDEN_PAGE? 0: 1;

		return 1;
	}
}