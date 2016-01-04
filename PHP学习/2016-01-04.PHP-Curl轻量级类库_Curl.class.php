<?php 
/**
 * note:还未测试
 * 
 * PHP-Curl轻量级类库
 *
 * 参考 https://github.com/wenpeng/PHP-Curl
 *
 */

class Curl
{
	const SUCCESS        = 0x000;
	const POST_ERROR     = 0x001;
	const DOWNLOAD_ERROR = 0x002;

	/**
	 * post
	 * @var array
	 * 
	 */
	private $post;

	/**
	 * [$retry description]
	 * @var int
	 */
	private $retry;

	/**
	 * [$option description]
	 * @var array
	 */
	private $option;

	/**
	 * [$default description]
	 * @var array
	 */
	private $default;

	/**
	 * [$download description]
	 * @var boolean
	 */
	private $download;

	public function __construct()
	{
		$this->retry = 0;
		$this->default = array(
			'CURLOPT_TIMEOUT'        => 30,
			'CURLOPT_ENCODING'       => '',
			'CURLOPT_IPRESOLVE'      => 1,
			'CURLOPT_RETURNTRANSFER' = true,
			'CURLOPT_SSL_VERIFYPEER' => false,
			'CURLOPT_CONNECTTIMEOUT' => 10,
		);
	}

	/**
	 * 提交GET请求
	 * 
	 * @param  string $url
	 * @return array
	 * 
	 */
	public function get($url)
	{
		return $this->set('CURLOPT_URL', $url)->exec();
	}

	/**
	 * 设置POST信息
	 * 
	 * @param  array|string $data
	 * @param  string $value
	 * @return $this
	 * 
	 */
	public function post($data, $value = '')
	{
		if (is_array($data))
		{
			foreach ($data as $key => &$value) 
			{
				$this->post[$key] = $value;
			}
		}
		else
		{
			$this->post[$data] = $value;
		}
		return $this;
	}

	/**
	 * 设置文件上传
	 * 
	 * @param string $field
	 * @param  string $path
	 * @param  string $type
	 * @param  string $name
	 * @return $this
	 * 
	 */
	public function upload($field, $path, $type, $name)
	{
		$name = basename($name);
		if (class_exists('CURLFile'))
		{
			$this->set('CURLOPT_SAFE_UPLOAD', true);
			$file = curl_file_create($path, $type, $name);
		}
		else
		{
			$file = sprintf("@{$s};type={%s};filename={%s}", $path, $type, $name);
		}

		return $this->post($field, $file);
	}

	/**
	 * 提交POST请求
	 * 
	 * @param  [type] $url [description]
	 * @return [type]      [description]
	 * 
	 */
	public function submit($url)
	{
		if (!$this->post)
		{
			return array(
				'error'   => self::POST_ERROR,
				'message' => '未设置POST信息',
			);
		}

		return $this->set('CURLOPT_URL', $url)->exec();
	}

	/**
	 * 设置下载地址
	 * 
	 * @param  [type] $url [description]
	 * @return [type]      [description]
	 * 
	 */
	public function download($url)
	{
		$this->download = true;
		return $this->set('CURLOPT_URL', $url);
	}

	/**
	 * 下载保存文件
	 * 
	 * @param  [type] $path [description]
	 * @return [type]       [description]
	 * 
	 */
	public function save($path)
	{
		if (!$this->download)
		{
			return array(
				'error'   => self::DOWNLOAD_ERROR,
				'message' => '未设置下载地址',
			);
		}

		$result = $this->exec();

		if ($result['error'] === self::SUCCESS)
		{
			$fp = fopen($path, 'w');
			fwrite($fp, $result['body']);
			fclose($fp);
		}

		return $result;
	}

	/**
	 * 配置curl操作
	 * 
	 * @param [type] $item  [description]
	 * @param string $value [description]
	 * 
	 */
	public function set($item, $value = '')
	{
		if (is_array($item))
		{
			foreach ($item as $key => &$value)
			{
				$this->option[$key] = $value;
			}
		}
		else
		{
			$this->option[$item] = $value;
		}

		return $this;
	}

	/**
	 * 出错自动重试
	 * 
	 * @param  integer $times [description]
	 * @return [type]         [description]
	 * 
	 */
	public function retry($times = 0)
	{
		$this->retry = $times;
		return $this;
	}

	/**
	 * 执行curl操作
	 * 
	 * @param  integer $retry [description]
	 * @return [type]         [description]
	 * 
	 */
	private function exec($retry = 0)
	{
		$ch = curl_init();

		$options = array_merge($this->default, $this->options);
		foreach ($options as $key => $val)
		{
			if (is_string($key))
				$key = constant(strtoupper($key));
			curl_setopt($ch, $key, $val);
		}

		if ($this->post)
		{
			curl_setopt($ch, CURLOPT_POST, true);
			curl_setopt($ch, CURLOPT_POSTFIELDS, $this->build_post_data($this->post));
		}

		$body  = curl_exec($ch);
		$info  = curl_getinfo($ch);
		$errno = curl_errno($ch);

		if ($errno === 0 && $info['http_code'] >= 400)
			$errno = $info['http_code'];

		curl_close($ch);

		//自动重试
		if ($errno && $retry < $this->retry)
			$this->exec($retry + 1);

		$this->post     = null;
		$this->retry    = null;
		$this->option   = null;
		$this->download = null;

		return array(
			'body'  => $body,
			'info'  => $info,
			'error' => $errno
		);
	}

	/**
	 * 扁平化POST信息
	 * 
	 * @param  [type] $input [description]
	 * @param  [type] $pre   [description]
	 * @return [type]        [description]
	 * 
	 */
	private function build_post_data($input, $pre = null)
	{
		if (is_array($input))
		{
			static $output = array();

			foreach ($input as $key => $value)
			{
				$index = is_null($pre) ? $key : "{$pre}[{$key}]";
				if (is_array($value))
					$output = array_merge($output, $this->build_post_data($value, $index));
				else
					$output[$index] = $value;
			}

			return $output;
		}

		return $input;
	}
}
 ?>