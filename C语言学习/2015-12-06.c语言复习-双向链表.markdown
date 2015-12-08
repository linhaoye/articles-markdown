C语言复习-链表1

---
```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Dlink_
{
	struct Dlink_ *prev;
	struct Dlink_ *next;
} Dlink;

typedef struct Vector_
{
	Dlink link;
	int len;
	char buf[128];
} Vector;


int main(void)
{
	char buffer[128]; 
	Vector *head = NULL;
	Vector *tail = NULL;

	do
	{
		printf("$please enter a string: ");
		scanf("%s", buffer);

		Vector *vec;

		if ( (vec = (Vector *)malloc(sizeof(Vector))) == NULL )
			return -1;

		memset(vec, 0, sizeof(Vector));

		vec->len = strlen(buffer);
		strcpy(vec->buf, buffer);
		vec->link.prev = NULL;
		vec->link.next = NULL;

		if (head == NULL)
		{
			head = vec;
		}
		else
		{
			Vector *tmp;
			Dlink *last = &head->link;
			while(last->next)	
				last = last->next;

			//获取最后一个节点的首地址
			tmp = (Vector *) ( (char*)(last) - (unsigned long) (&( (Vector*) 0)->link));

			tmp->link.next = &vec->link;
			vec->link.prev = &tmp->link;
		}
		tail = vec;

	} while(strcmp(buffer, "quit") != 0);


	//前向遍历链表
	Vector *ncursor = NULL;
	Dlink *next = &head->link;

	printf("\r\n$begin travel forward chains!\r\n");
	while(next)
	{
		ncursor = (Vector *) ( (char*)(next) - (unsigned long) (&((Vector*)0)->link));
		printf("$printf node information: (len: %3d, value: %s).\r\n", ncursor->len, ncursor->buf);
		next = next->next;
	}
	printf("\r\n$end travel forward chains!\r\n");


	//后向遍历链表
	Vector *bcursor = NULL;
	Dlink *prev = &tail->link;

	printf("\r\n$begin travel backward chains!\r\n");
	while(prev)
	{
		bcursor = (Vector *) ( (char*)(prev) - (unsigned long) (&((Vector*)0)->link));
		printf ("$printf node information: (len: %3d, value: %s).\r\n", bcursor->len, bcursor->buf);
		prev = prev->prev;
	}
	printf("\r\n$end travel backward chains!\r\n");

	//前向删除节点
	Vector *delnode = NULL;
	Dlink *cnext = &head->link;

	printf("\r\n$begin release node!\r\n");
	while(cnext)
	{
		delnode = (Vector *) ( (char*)(cnext) - (unsigned long) (&((Vector*)0)->link));

		if (delnode)
		{
			printf("$release node: (len: %3d, value: %s)\r\n", delnode->len, delnode->buf);
			cnext = delnode->link.next;
			free(delnode);
		}
	}
	printf("\r\n$end release node!\r\n");

	return 0;
}
```