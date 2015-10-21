#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/epoll.h>


#define _DEBUG

/* revent 事件宏定义 
 *
 * 相应的操作可以使用 | 运算来并几个需要监听的事件类型
 */ 
#define MC_EV_READ      0x0001
#define MC_EV_WRITE     0x0002
#define MC_EV_SIGNAL    0x0004
#define MC_EV_TIMEOUT   0x0008
#define MC_EV_LISTEN    0x0010

/* mc_event_中, ev_flags是事件的状态, 宏定义如下 */

#define MC_EV_INITD     0x0001      /* 初始化 */
#define MC_EV_ADDED     0x0002      /* 已加入队列 */
#define MC_EV_ACTIVE    0x0004      /* 已加入激活队列 */
#define MC_EV_DELED     0x0008      /* 已删除 */

#define MC_BASE_MAGIC   0x2015

typedef void (*mc_ev_callback)(int fd, short revent, void *args);

/* 反应堆结构*/
typedef struct mc_event_base_
{
	void        *   added_list;
	void        *   active_list;
	unsigned int    event_num;          //事件个数
	unsigned int    event_actvie_num;   //已激活事件个数
	int             epoll_fd;           //epoll_create创建的epoll例程
	int             ev_base_stop;       //判断是否停止的标志位
	int             magic;              //定义为一个宏
	struct timeval  event_time;
} mc_event_base_t;

mc_event_base_t * mc_base_new(void);

/*事件类型定义*/
typedef struct mc_event_
{
	struct          mc_event_ *next;
	struct          mc_event_ *prev;
	unsigned int    min_heap_index;     //超时管理的最小堆的下标
	int             ev_fd;              //发生事件的文件描述符
	short           revent;             //事件类型
	struct timeval  ev_timeval;         //事件超时时间
	mc_ev_callback  callback;           //事件处理回调函数
	void            *args;
	int             ev_flags;
	mc_event_base_t *base;
} mc_event_t;

/* 为监听的文件描述符加入回调函数, 并注册事件类型 */
int mc_event_set(mc_event_t *ev, short revent, int fd, mc_ev_callback callback, void *args);
/* 投递事件, 将需要监听的并且已经初始化的事件加入反应堆*/
int mc_event_post(mc_event_t *ev, mc_event_base_t *base);
/* 事件路由, 反应开始循环, 等待事伯的发生 */
int mc_dispatch(mc_event_base_t *base);

/* 为事件封装的操作 */
typedef struct mc_event_opt_
{
	void * (*init)(mc_event_base_t *);                              //初始化
	int    (*add)(void *, mc_event_t *);                            //加入队列
	int    (*del)(void *, mc_event_t *);                            //删除事件
	int    (*mod)(void *, mc_event_t *);                            //修改事件
	int    (*dispatch)(void *, mc_event_base_t *, struct timeval);  //循环监听事件
} mc_event_opt;

extern mc_event_opt mc_event_op_val;

#define mc_event_init    mc_event_op_val.init;
#define mc_event_add     mc_event_op_val.add;
#define mc_event_del     mc_event_op_val.del;
#define mc_event_mod     mc_event_op_val.mod;
#define mc_event_loop    mc_event_op_val.mc_dispatch;

static void * mc_epoll_init(mc_event_base_t *meb);
static int mc_epoll_add(void *arg, mc_event_t *ev);
static int mc_epoll_del(void *arg, mc_event_t *ev);
static int mc_epoll_mod(void *arg, mc_event_t *ev);
static int mc_epoll_loop(void *arg, mc_event_t *ev);

mc_event_op_val = {
	mc_epoll_init,
	mc_epoll_add,
	mc_epoll_del,
	mc_epoll_mod,
	mc_epoll_loop,
};


#define mc_sock_fd          int
#define DEFAULT_NET         AF_NET
#define DEFAULT_DATA_GRAM   SOCK_STREAM
#define DEFAULT_PORT        12345
#define DEFAULT_BACKLOG     200
#define MC_EVENT_MAX        1000

/*
 * 定义一个connection结构, 用于表示每一个到来的连接
 */
typedef struct connection_
{
	int                 fd;         //注册在反应堆的fd
	mc_event_t          read;       //读事件
	mc_event_t          write;      //写事件
	char                buf[1024];  //缓冲区
	mc_event_base_t     *base;      //指向反应堆的指针
} mc_connection;

#define LOG_OUTPUT(std, fmt, ...) do {                                                        \
			time_t t = time(NULL);                                                      \
			struct tm *dm = localtime(&t);                                              \
																						\
			fprintf(std, "[%02d:%02d:%02d] %s:[" _QUOTE(__LINE__) "]\t       %-26%:")\
					fmt "\n", __FILE__, dm->tm_hour, dm->tm_min, dm->tm_sec, __func__,  \
					## __VA_ARGS__);                                                    \
			fflush(stdout);                                                             \
} while(0)

#ifdef _DEBUG
#define __QUOTE(x)      # x
#define  _QUOTE(x)      _QUOTE(x)
#define LOG_DEBUG(fmt, ...) LOG_OUTPUT(stdout, fmt, ...)
#endif

#define LOG_ERROR(fmt, ...) LOG_OUTPUT(stderr, fmt, ...)


static void setreuseaddr(mc_sock_fd fd);
static int mc_socket();
static int mc_bind(mc_sock_fd listenfd);
static int mc_listen(mc_sock_fd listenfd);
static void handler_accept(int fd, short revent, void *args);
static void handler_read(int fd, short revent, void *args);
static void hnadler_write(int fd, short revent, void *args);
static void cab(int fd, short revent, void *args);


int main(int argc, char const *argv[])
{
	mc_event_t mev;
	mc_event_base_t *base = mc_base_new();
	mc_connection lc;

	int sockfd = mc_socket();
	mc_bind(sockfd);
	mc_listen(sockfd);

	mc_event_set(&(lc.read), MC_EV_READ, sockfd, handler_accept, &lc);
	mc_event_post(&(lc.read), base);
	mc_dispatch(base);

	return 0;
}

mc_event_base_t * mc_base_new(void)
{
	mc_event_base_t * base = (mc_event_base_t *)malloc( sizeof(mc_event_base_t) );
	if( base == NULL ) {
		LOG_DEBUG("Init the base moudle in mc_base_new error in file");
		return NULL ;
	}
	 
	/* init the base lists */
	base->added_list  = NULL  ;
	base->active_list = NULL ;
	 
	base->magic = MC_BASE_MAGIC ;
		 
	base->event_num        = 0 ;
	base->event_active_num = 0 ;
	 
	base->ev_base_stop = MC_BASE_STOP  ;
	 
	base->magic = MC_BASE_MAGIC ;
	 
	gettimeofday(&base->event_time,NULL);
	 
	mc_event_init( base ) ;
	return base ;
	 
}
 
int mc_event_set( mc_event_t *ev , short revent , int fd , mc_ev_callback callback , void *args )
{
	if( ev == NULL )
	{
		LOG_DEBUG(stderr, " mc_event_set error , ev == NULL or other segment error in file:%s,line:%d\n",__FILE__,__LINE__);
		return -1 ;
	}
	#if (HAVE_EPOLL)   
	unsigned int epoll_flag ;
	#endif
	int err ;
	memset(ev,0,sizeof(mc_event_t));
	ev->revent = revent ;
	ev->ev_fd     = fd   ;
	ev->callback = callback ;
	ev->next = NULL ;
	ev->prev = NULL ;
	if( args == NULL )
		ev->args = NULL  ;
	else
		ev->args = args  ;  
	 
	/* This job post to mc_event_post
	 *if( revent & MC_EV_LISTEN )
	 *{
	 *  err = mc_event_add(NULL , ev );
	 *  if( err != 0 )
	 *      LOG_DEBUG(stderr,"mc_event_add in mc_event_set \n");
	 *}
	 */
	  
	/* event should post to base */
	if( ev->base == NULL )
		return 0;
		 
	#if (HAVE_EPOLL)   
	 
	 
		if( revent & MC_EV_READ )
		{
			epoll_flag = EPOLLIN|EPOLLET ;
		 
			err = mc_event_mod( (void *)&epoll_flag , ev ) ;
			if( err != 0 )
				LOG_ERROR("mc_event_mod (MC_EVENT_READ ) in mc_event_set");
		}
	 
		if( revent & MC_EV_WRITE )
		{
			epoll_flag = EPOLLOUT|EPOLLET ;
			err = mc_event_mod( (void *)&epoll_flag ,ev );
			if( err != 0 )
				LOG_DEBUG(stderr,"mc_event_mod (MC_EVENT_WRITE) in mc_event_set in file:%s,line:%d\n",__FILE__,__LINE__);
		}
	 
		ev->ev_flags |= MC_EV_INITD  ;
		#endif
	 
		return 0 ;
}
 
int mc_event_post( mc_event_t *ev , mc_event_base_t * base )
{
	if( ev == NULL || base == NULL )
	{
		LOG_DEBUG(stderr," In function mc_event_post , the args error , please check your arguments in file:%s,line:%d\n",__FILE__,__LINE__);
		return -1;
	}
	if( base->magic != MC_BASE_MAGIC )
	{
		LOG_DEBUG(stderr,"The mc_event_base_t * points base non inited in mc_event_post in file:%s,line:%d\n",__FILE__,__LINE__);
		return -1;
	}
	int err ;
	ev->base = base ;
	add_event_to_queue(ev,(mc_event_t **)&(base->added_list));
	base->event_num++;
	 
	err = mc_event_add( NULL , ev );
	if( err == -1 )
	{
		LOG_DEBUG(stderr,"In function mc_event_add error in file:%s,line:%d\n",__FILE__,__LINE__);
		return -1;
	}
}
 
 
 
int mc_dispatch( mc_event_base_t * base )
{
	if( base == NULL )
	{
		LOG_DEBUG(stderr, "base == NULL in function mc_dispatch in file:%s,line:%d\n",__FILE__,__LINE__);
		return -1 ;
	}
	if( base->magic != MC_BASE_MAGIC )
	{
		LOG_DEBUG(stderr,"In function mc_disptahch noinitlized line:%d , in file:%s ",__LINE__,__FILE__);
		return -1 ;
	}
	 
	struct epoll_event *nevents = ( struct epoll_event *)malloc( sizeof( struct epoll_event ) );
	int done = 0 ;
	 
	int nevent ;
	int i ;
	mc_event_t *levent ;
	mc_event_t *retevent ;
	while( !done )
	{
		nevent = mc_event_loop( nevents , base , base->event_time) ;
		 
		if( nevent == -1 )
		{
			LOG_DEBUG(stderr,"No event check , return in file:%s line:%d \n",__FILE__,__LINE__);
			goto err1;
		}
		for( i = 0 ; i < nevent ; i++ )
		{
			if( nevents[i].events & EPOLLERR || nevents[i].events & EPOLLHUP )
			{
				levent = nevents[i].data.ptr ;
				if( !(levent->ev_flags & MC_EV_INITD) )
					continue ;
				if( (levent->ev_flags & MC_EV_ACTIVE) || (levent->ev_flags & MC_EV_ADDED ) )
					del_event_from_queue( levent );
			}
			if( nevents[i].events & EPOLLIN )
			{
				levent = nevents[i].data.ptr ;
				levent->revent = MC_EV_READ  ;
				add_event_to_queue( levent , (mc_event_t **)&(base->active_list) ); 
				levent->ev_flags |=  MC_EV_ACTIVE ; 
				base->event_active_num++;   
			}
			else if(nevents[i].events & EPOLLOUT)
			{
				levent = nevents[i].data.ptr ;
				levent->revent = MC_EV_WRITE ;
				add_event_to_queue( levent , (mc_event_t **)&(base->active_list) ); 
				levent->ev_flags |=  MC_EV_ACTIVE ; 
				base->event_active_num++;
			}
			else
			{
				LOG_DEBUG(stderr,"Unknow err in file:%s,line:%d\n",__FILE__,__LINE__);
				goto err1;
			}
		}
		 
		retevent  = (mc_event_t *)(base->active_list) ;
		for(i = 0 ;i < nevent ; i++ )
		{  
			LOG_DEBUG(stderr," %d event(s)\n",nevent);
			if( retevent == NULL )
				break ;
		 
			retevent = get_event_and_del( (mc_event_t *)(base->active_list) ) ;
			/* If we want to reuse this event we should set event again */
			retevent->ev_flags = retevent->ev_flags&(~MC_EV_ACTIVE);
			base->event_active_num-- ;
			if( retevent == NULL )
				LOG_DEBUG(stderr,"event is NULL file:%s,line:%d\n",__FILE__,__LINE__);   
	 
			retevent->callback(retevent->ev_fd,retevent->revent,retevent->args) ;
		}
		 
	}
		return  0 ;
	err1:
		return -1 ;
}

#define ASSERT_BASE_MAGIC(b, m) do {         \
			if (b->magic != MC_BASE_MAGIC) { \
				LOG_DEBUG(m);                \
				return -1;                   \
			}                                \
} while(0)

static void *mc_epoll_init(mc_event_base_t *meb)
{
	if (meb->magic != MC_BASE_MAGIC) {
		LOG_DEBUG("init error");
		return NULL;
	}
	meb->epoll_fd = epoll_create(MC_EVENT_MAX);

	return meb;
}

static int mc_epoll_add(void *arg, mc_event_t *ev)
{
	ASSERT_BASE_MAGIC(ev->base, "add error");

	mc_event_base_t *base = ev->base;
	int epoll_fd = base->epoll_fd;
	int err;

	struct epoll_event epoll_ev;
	epoll_ev.data.ptr = ev;
	epoll_ev.events = EPOLLIN | EPOLLET;

	if (!(ev->ev_flags & MC_EV_ADDED)) {
		err = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, ev->ev_fd, &epoll_ev);

		if (err != 0) {
			LOG_DEBUG("epoll_ctl error");
			return -1;
		}
		ev->ev_flags |= MC_EV_ADDED;
	}
	return 0;
}

int mc_epoll_del( void * arg ,mc_event_t *ev )
{
	ASSERT_BASE_MAGIC(ev->base, "add error");

	mc_event_base_t *base = ev->base ;
	int epoll_fd = base->epoll_fd ;
	int err ;
	 
	if( !(ev->ev_flags & MC_EV_INITD) ) {
		return -1 ;
	}
	err = epoll_ctl( epoll_fd , EPOLL_CTL_DEL , ev->ev_fd , NULL );

	if( err != 0 ) {
		LOG_DEBUG("epoll_ctl error");
		return -1;
	}
	ev->ev_flags = 0x0000 ;
	 
	return 0;
}

int mc_epoll_mod(void * arg ,mc_event_t *ev )
{
	ASSERT_BASE_MAGIC(ev->base, "add error");

	mc_event_base_t *base = ev->base ;
	int epoll_fd = base->epoll_fd ;
	int err ;
	unsigned int mode ;

	if( !(ev->ev_flags & MC_EV_INITD) ) {
		return -1 ;
	}
	 
	struct epoll_event epoll_ev  ;     
	epoll_ev.data.ptr = ev ;

	if( arg == NULL ) {
		return 0;
	}

	mode = *(unsigned int *)arg ;
	epoll_ev.events = mode ;
	err = epoll_ctl( epoll_fd , EPOLL_CTL_MOD , ev->ev_fd , &epoll_ev );

	if( err != 0 ) {
		LOG_DEBUG("epoll_ctl error");
		return -1;
	}
	return 0;
}

int  mc_epoll_loop( void * args , mc_event_base_t *base , struct timeval ev_time )
{
	if( base == NULL ) {
		LOG_DEBUG("base == NULL");
		return -1 ;
	}
	
	ASSERT_BASE_MAGIC("loop error");     
	 
	int nfds ;
	 
	/* we pass args as nevents in this function */
	struct epoll_event *nevents = ( struct epoll_event * )args ;
	 
	struct epoll_event epoll_ev  ; 
	nfds = epoll_wait(  base->epoll_fd , nevents , MC_EVENT_MAX , 1) ;

	if( nfds <= -1 ) {
		LOG_DEBUG("epoll wait error");
		return nfds ;
	}
	return nfds ;
}