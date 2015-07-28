前些时间,同事问起我一个问题, 就是能不能通过SQL语句将一个表的某些字段记录导入别个表去,
我稍微想想, 这不就是MySQLM表复制的知识吗.当时我记得不太清楚了,便叫上网查查MySQL表复制,
后来自己日后可能会用到, 便复习一下.

1. 复制表结构及数据到新表

```sql
CREATE TABLE new_table SELECT * FROM old_table
```

这种方法会将old_table中所有的内容都拷贝过来, 当然我们可以用`DELETE FROM new_table`来删除.
不过这种方法的一个最不好的地方就是新表中没有了旧表的PRIMARY KEY、Extra(AUTO_INCREMENT)等属性.
需要自己用`ALTER`添加而且容易搞错.

2. 只复制表结构到新表

```sql
CREATE TABLE new_table SELECT * FROM old_table WHERE 1 = 2

#或者

CREATE TABLE new_table LIKE old_table
```

3. 复制旧表的数据到新表(假设两个表结构一样)

```sql
INSERT INTO new_table SELECT * FROM old_table
```

4. 复制旧表的数据到新表(假设两个表结构不一样)

```sql
INSERT INTO new_table(field1, field2, ...) SELECT field1,field2,... FROM old_table
```

5. 可以将表1结构复制到表2

```sql
SELECT * INTO new_table FROM old_table WHERE 1 = 2
```

6. 可以将表1内容全部复制到表2

```sql
SELECT * INTO table1 FROM table2
```

7. SHOW　CREATE TABLE old_table

```
这样会将旧表的创建命令列出. 我们只需要将该命令拷贝出来, 更改table的名字, 就可以建立一个完全一样的表.
```

8. Mysqldump

```
用mysqldump将表dump出来, 改名字后再导回去或者直接在命令行中运行.
```