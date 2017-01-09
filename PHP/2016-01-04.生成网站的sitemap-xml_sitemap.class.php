<?php 
class sitemap
{
	/**
	 * @__construct, hmn
	 *
	 */
	public function __construct()
	{
		$this->ci            = & get_instance();
		$this->_sitemap_file = GMLPATH."/sitemap.xml";

		$this->write_sitemp_file();
	}

	/**
	 * 输出sitemap xml
	 *
	 * @return void
	 *
	 */
	public function output_sitemap()
	{
		header('Content-type: application/xml');
		echo $this->_get_sitemap();
	}

	/**
	 * 写入sitemap 文件
	 *
	 * @return void
	 *
	 */
	public function write_sitemp_file()
	{
		if ($this->_sitemap_expired())
		{
			/* 已过期, 重新生成 */
			$updated_items = $this->_get_updated_items($this->_get_sitemap_lastupdate());

			$sitemap = $this->_build_sitemap($updated_items);

			$this->_write_sitemap($sitemap);
		}
	}

	/**
	 * 即时写入sitemap文件
	 * 
	 * @return void
	 *
	 */
	public function immediately_write_sitemap()
	{
		$updated_items = $this->_get_updated_items();
		$sitemap       = $this->_build_sitemap($updated_items);	

		$this->_write_sitemap($sitemap);

		$this->output_sitemap();
	}

	/**
	 * 取得sitemap
	 * 
	 * @return string
	 * 
	 */
	private function _get_sitemap()
	{
		$sitemap = "";
		if ($this->_sitemap_expired())
		{
			/* 已过期, 重新生成 */
			$updated_items = $this->_get_updated_items($this->_get_sitemap_lastupdate());

			$sitemap = $this->_build_sitemap($updated_items);

			$this->_write_sitemap($sitemap);
		}
		else
		{
			$sitemap = file_get_contents($this->_sitemap_file);
		}

		return $sitemap;
	}

	/**
	 * 判断是过期
	 *
	 * @return int
	 *
	 */
	private function _sitemap_expired()
	{
		if (!is_file($this->_sitemap_file))
		{
			return true;
		}
		$frequency = 12 * 3600;
		$filemtime = $this->_get_sitemap_lastupdate();

		return (time() >= $filemtime + $frequency);
	}

	/**
	 * 获取上次更新时间
	 *
	 * @return int
	 *
	 */
	private function _get_sitemap_lastupdate()
	{
		return is_file($this->_sitemap_file) ? filemtime($this->_sitemap_file): 0;
	}


	/**
	 * 获取更新item
	 *
	 * @param  int
	 * @return array
	 *
	 */
	private function _get_updated_items($timeline = 0)
	{
		$timeline && $timeline -= date('Z');
		$limit = 5000;
		$result = array();

		/* 更新文章 */
		$articles = $this->ci->db->query("SELECT * FROM ". $this->ci->db->dbprefix("news")." ORDER BY create_on DESC")->result();

		if (!empty($articles))
		{
			foreach ($articles as $item)
			{
				$result[] = array(
					'url'        => site_url("news/news_detail/".$item->id),
					'lastmod'    => date("Y-m-d", $item->create_on),
					'changefreq' => 'daily',
					'priority'   => '0.8',
				);	
			}
		}

		/* 更新百科 */
		$baikes = $this->ci->db->query("SELECT * FROM ". $this->ci->db->dbprefix("baike")." ORDER BY create_on DESC")->result();

		if (!empty($baikes))
		{
			foreach ($baikes as $item)
			{
				$result[] = array(
					'url'        => site_url("baike/baike_detail/".$item->id),
					'lastmod'    => date("Y-m-d", $item->create_on),
					'changefreq' => 'daily',
					'priority'   => '0.8',
				);	
			}
		}

		return $result;
	}

	/**
	 *
	 * 生成 sitemap xml
	 *
	 * @param  array $items
	 * @return string
	 *
	 */
	private function _build_sitemap($items)
	{
        $sitemap = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n<urlset xmlns=\"http://www.sitemaps.org/schemas/sitemap/0.9\">\r\n";
        $sitemap .= "    <url>\r\n        <loc>" . htmlentities(site_url(), ENT_QUOTES) . "</loc>\r\n        <lastmod>" . date('Y-m-d', time()) . "</lastmod>\r\n        <changefreq>always</changefreq>\r\n        <priority>1</priority>\r\n    </url>";
        if (!empty($items))
        {
            foreach ($items as $item)
            {
                $sitemap .= "\r\n    <url>\r\n        <loc>" . htmlentities($item['url'], ENT_QUOTES) . "</loc>\r\n        <lastmod>{$item['lastmod']}</lastmod>\r\n        <changefreq>{$item['changefreq']}</changefreq>\r\n        <priority>{$item['priority']}</priority>\r\n    </url>";
            }
        }
        $sitemap .= "\r\n</urlset>";

        return $sitemap;
	}

	/**
	 *
	 * 写入sitemap文件
	 *
	 * @param  string $sitemap
	 * @return void
	 *
	 */
	private function _write_sitemap($sitemap)
	{
		chmod($this->_sitemap_file, 0777);
		file_put_contents($this->_sitemap_file, $sitemap, LOCK_EX);
	}
}
 ?>