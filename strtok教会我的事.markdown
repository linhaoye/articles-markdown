strtok函数分解字符串为一组标记串，原型为：extern char *strtok(char *s, char *delim);

s为要分解的字符串，delim为分隔符字符串。首次调用时，s必须指向要分解的字符串，随后调用要把s设成NULL,strtok在s中查找包含在delim中的字符并用NULL(‘\0’)来替换，直到找遍整个字符串。返回指向目前找到的最后一个标记串，当没有标记串时则返回字符NULL。

看下面的一段程序:

```c
main()
{
	char *s="Golden Global View";
	char *d=" ";
	char *p;
 
	p=strtok(s,d);
	while(p)
	{
		printf("%s\n",p);
		p=strtok(NULL,d);
	}
}
```
编译没有问题，但运行时会出现Segmentation fault的错误.

有些人就会百思不得其解，认为是strtok的问题，其实不然。

如果将s的定义由char *s改成char s[]或char s[20]，则运行时就不会出现段错误而是想要的结果。可见问题不是strtok而是字符串常量的问题。
看strtok的原型，s不是const的，说明在strtok里会修改s的值，而如果定义char *s="Golden Global View"，则s指向只读区域不能修改。

可以看下面的程序:

```c
#include
 
int main()
{
	char *s=(char *)malloc(sizeof(char)*10);
	s="abcdef";
	*(s+1)='g';
}
```