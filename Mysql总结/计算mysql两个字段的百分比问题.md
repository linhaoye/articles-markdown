最近朋友问到我一个问题, 问题是这样的:
```SQL
#他数据库有张表其中2个字段, 会员价和分销价, 取出记录后,想让记录按照 会员价和分销价的百分比来排序.
#比如如下表:

CREATE TABLE products (
	id unsigned int auto increment not null
	...
	member_price float,
	hot_proice float,
	...
) ENGINE = MyISAM

```
正常情况下, 我们都想在取出数据后, 再来做数据处理然后排序,比如用PHP的uasort或usort或array_mutilsort 后来想到简便的一个方法, 就是用mysql函数方法来处理.

解决办法如下:

```SQL
SELECT
	id,
	member_price,
	hot_price,
	CONCAT(member_price / hot_price * 100, 5),
	'%'
	) AS percentage
FROM
	products
ORDER BY
	percentage

```

其实这种方法虽说方便, 但是也有坏处, 就是会影响mysql的性能.