#sublime text编译程序时提示错误[Decode error - output not utf-8] compilation terminated

在st2里写了一个c程序, 如下:

```c
#include "vec.h"

/* @test */
main(void)
{
    Vec(int) array;
    vec_init(&array);
    vec_push(&array, 34);
    vec_push(&array, 12);
    vec_push(&array, 'c');

    printf("total length = %d\n", array.length);

    int inum;
    printf("array get %c\n", vec_get(&array, 2));
    inum = vec_pop(&array);
    printf("inum = %d\n", inum);
    printf("array length = %d\n", array.length);

    int fnum;
    fnum = vec_pop(&array);
    printf("fnum = %d\n", fnum);
    printf("array length = %d\n", array.length);

    char ch;
    ch = vec_pop(&array);
    printf("ch = %d\n", ch);
    printf("array length = %d\n", array.length);


    vec_deinit(&array);
}
```

经过百度, 找到了一些针对python和java程序遇到这个问题的解决办法.
不过我的程序毕竟是c, 但我仍然找到了一些相同之处, 通过试验果然解决掉了.

解决办法如下:

1、首先在Preferences里点击Browse Packages：
`Preferences`->`Browse Packages...`

2、然后在里面找到User, 点击进入

3、找到C.sublime-build, 点击打开文件

```javascript
{
	"cmd": ["g++", "${file}", "-o", "${file_path}/${file_base_name}"],
	"file_regex": "^(..[^:]*):([0-9]+):?([0-9]+)?:? (.*)$",
	"working_dir": "${file_path}",
	"selector": "source.c, source.c++",

	"variants":
	[
		{
			"name": "Run",
			"cmd": ["cmd", "/c", "g++", "${file}", "-o", "${file_path}/${file_base_name}", "&&", "cmd", "/c", "${file_path}/${file_base_name}"]
		},
		{
			"name": "RunInCommand",
			"cmd": ["cmd", "/c", "g++", "${file}", "-o", "${file_path}/${file_base_name}", "&&", "start", "cmd", "/c", "${file_path}/${file_base_name} & pause"]
		}
	]
}
```

在文件的这个文本的"selector": "source.c, source.c++",下一行添加一个语句：
"encoding":"cp936",
注意有逗号
然后保存后，重启ST2，问题就解决啦