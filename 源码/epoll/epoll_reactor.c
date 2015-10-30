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
#define HAVE_EPOLL      1

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


#define MC_BASE_STOP    0x0000
#define MC_BASE_ACTIVE  0x0001
#define MC_BASE_MAGIC   0x2015

#define MC_EVENT_MAX    1000

/* 双向链表的宏定义 */
 struct llhead {
    struct llhead *prev, *next;
 };

 #define LL_INIT(N)         ((N)->next = (N)->prev = (N))
 #define LL_HEAD(H)         struct llhead H = {&H, &H}
 #define LL_ENTRY(P,T,N)    ((T *)((char *)(P) - (unsigned long)(&((T *) 0)->N)))

 #define LL_PUSH(H, N)      do {        \
            (H)->next->prev = (N);      \
            (N)->next = ((H)->next);    \
            (H)->next->prev = (H);      \
            (H)->next = (N);            \
} while(0)

 #define LL_TAIL(H, N)      do {        \
            ((H)->prev)->next = (N);    \
            (N)->prev = ((H)->prev);    \
            (N)->next = (H);            \
            (H)->prev = (N);            \
} while(0)

#define LL_DEL(N)           do {            \
            ((N)->next)->prev = ((N)->prev);\
            ((N)->prev)->next = ((N)->next);\
            LL_INIT(N);                     \
} while(0)
            

#define LL_FOREACH(H, N) for (N = (H)->next; N != (H); N = (N)->next)

#define LL_FOREACH_SAFE(H, N, T)                         \
            for (N = (H)->next, T = (N)->next; N != (H); \
                N = (T), T = (N)->next)

typedef void (*mc_ev_callback)(int, short, void *);

/* 反应堆结构*/
typedef struct mc_event_base_
{
    struct llhead * added_list;
    struct llhead * active_list;
    unsigned int    event_num;          //事件个数
    unsigned int    event_actvie_num;   //已激活事件个数

#if (HAVE_EPOLL)
    int             epoll_fd;           //epoll_create创建的epoll例程
#endif

    int             ev_base_stop;       //判断是否停止的标志位
    int             magic;              //定义为一个宏
    struct timeval  event_time;
} mc_event_base_t;

mc_event_base_t * mc_base_new(void);

/*事件类型定义*/
typedef struct mc_event_
{
    struct          llhead link;
    unsigned int    min_heap_index;     //超时管理的最小堆的下标
    int             ev_fd;              //发生事件的文件描述符
    short           revent;             //事件类型
    struct timeval  ev_timeval;         //事件超时时间
    mc_ev_callback  callback;           //事件处理回调函数
    void            *args;
    int             ev_flags;
    mc_event_base_t *base;
} mc_event_t;

/* 新建一个reactor结构体 */
mc_event_base_t *mc_base_new(void);
/* 为监听的文件描述符加入回调函数, 并注册事件类型 */
int mc_event_set(mc_event_t *ev, short revent, int fd, mc_ev_callback callback, void *args);
/* 投递事件, 将需要监听的并且已经初始化的事件加入反应堆*/
int mc_event_post(mc_event_t *ev, mc_event_base_t *base);
/* 事件路由, 反应开始循环, 等待事伯的发生 */
int mc_dispatch(mc_event_base_t *base);
/* 释放反应堆 */
void mc_base_dispose(mc_event_base_t * base);

//将事件加入队列
static void add_event_to_queue(mc_event_t *ev, void * queue);
//将事件从队列删除
static void del_event_from_queue(mc_event_t *ev);
//事件队列dequeue
static mc_event_t * get_event_and_del(void * queue);
//清空队列
static void destroy_queue_events(void * queue);
static void destroy_queue_events_safe(void *queue);
//打印事件列表
static void log_printf_events(void * queue);

/* 为事件封装的操作 */
typedef struct mc_event_opt_
{
    void * (*init)(mc_event_base_t *);                              //初始化
    int    (*add)(void *, mc_event_t *);                            //加入队列
    int    (*del)(void *, mc_event_t *);                            //删除事件
    int    (*mod)(void *, mc_event_t *);                            //修改事件
    int    (*dispatch)(void *, mc_event_base_t *, struct timeval);  //循环监听事件
} mc_event_opt;

static void * mc_epoll_init(mc_event_base_t *meb);
static int mc_epoll_add(void *arg, mc_event_t *ev);
static int mc_epoll_del(void *arg, mc_event_t *ev);
static int mc_epoll_mod(void *arg, mc_event_t *ev);
static int mc_epoll_loop(void *arg, mc_event_base_t *meb, struct timeval ev_timeval);

mc_event_opt mc_event_op_val = {
    mc_epoll_init,
    mc_epoll_add,
    mc_epoll_del,
    mc_epoll_mod,
    mc_epoll_loop
};

#define mc_event_init    mc_event_op_val.init;
#define mc_event_add     mc_event_op_val.add;
#define mc_event_del     mc_event_op_val.del;
#define mc_event_mod     mc_event_op_val.mod;
#define mc_event_loop    mc_event_op_val.dispatch;


#define DEFAULT_PORT        12345
#define DEFAULT_BACKLOG     200

/* vector的定义 */
struct vec
{
    int         len;
    const void  *ptr;
};

/* HTTP URL */
struct url
{
    struct vec  proto;
    struct vec  user;
    struct vec  pass;
    struct vec  host;
    struct vec  port;
    struct vec  uri;
};

/* HTTP 报头 */
struct hthdr
{
    struct vec name;
    struct vec value;
};

/* HTTP information */
struct hti
{
    struct vec      method;
    struct vec      url;
    struct hthdr    hh[64];     /* 报头*/
    int             nhdrs;
    int             reqlen;
    int             totlen;
};

struct sa
{
    socklen_t len;
    union {
        struct sockaddr     sa;
        struct sockaddr_in  sin;
    } u;

/* WITH_IPV6 */
#ifdef WITH_IPV6
    struct sockaddr_in6     sin6;
#endif 
};

struct iobuf
{
    mc_event_t  read;
    mc_event_t  write;
    char        buf[16 * 1024];
    int         nread;
    int         nwritten;
};

/*
 * 定义一个connection结构, 用于表示每一个到来的连接
 */
typedef struct connection_
{
    int                 fd;         //注册在反应堆的fd
    struct sa           sa;
    struct iobuf        remote;
    struct hti          hti;
    struct vec          uri;
    struct vec          req;
    struct vec          auth;
    struct vec          cookie;
    struct vec          clength;
    struct vec          ctype;
    struct vec          status;
    struct vec          location;
    time_t              ims;
    time_t              expire;
    char                user[32];
    int                 ntotal;
    int                 nexpected;

    char                gotrequest;
    char                gotreply;
    char                cgimode;
    char                sslaccepted;

    mc_event_base_t     *base;      //指向反应堆的指针

} mc_connection;

#define __QUOTE(x)      # x
#define  _QUOTE(x)      __QUOTE(x)

#define LOG_OUTPUT(std, fmt, ...) do {                                                  \
            time_t t = time(NULL);                                                      \
            struct tm *dm = localtime(&t);                                              \
                                                                                        \
            fprintf(std, "[%02d:%02d:%02d] %s:[" _QUOTE(__LINE__) "]\t       %-26s:"    \
                    fmt "\n", dm->tm_hour, dm->tm_min, dm->tm_sec, __FILE__, __func__,  \
                    ## __VA_ARGS__);                                                    \
            fflush(stdout);                                                             \
} while(0)

#ifdef _DEBUG
#define LOG_DEBUG(fmt, ...) LOG_OUTPUT(stdout, fmt, ## __VA_ARGS__)
#endif

#define LOG_ERROR(fmt, ...) LOG_OUTPUT(stderr, fmt, ## __VA_ARGS__)

static int blockmode(int fd, int block);
static int mc_listen(uint16_t listenfd);
static void handler_accept(int fd, short revent, void *args);
static void handler_read(int fd, short revent, void *args);
static void hnadler_write(int fd, short revent, void *args);
static void cab(int fd, short revent, void *args);

static int blockmode(int fd, int block)
{
    int flags, retval = 0; 

    if ((flags = fcntl(fd, F_GETFL, 0)) == -1)
    {
        LOG_ERROR("nonblock: fcntl(%d, F_GETFL): %s", fd, strerror(ERRNO));
        retval--;
    }
    else
    {
        if (block)
            flags &= ~O_NONBLOCK;
        else
            flags |= O_NONBLOCK;

        //Apply the mode
        if (fcntl(fd, F_SETFL, flags) != 0)
        {
            LOG_ERROR("nonblock: fcntl(%d, F_SETFL): %s", fd, strerror(ERRNO));
            retval--;
        }
    }

    return (retval);
}

static int mc_listen(uint16_t port)
{
    int sock, on = 1, af;
    struct sa sa;

#ifdef WITH_IPV6
    af = PF_INET6;
#else
    af = PF_INET;
#endif

    if ((sock = socket(af, SOCK_STREAM, 6)) == -1)
    {
        LOG_ERROR("Listen: socket: %s", strerror(ERRNO));
        return -1;
    }

    blockmode(sock, 0);
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *) &on, sizeof(on));

#ifdef WITH_IPV6
    sa.u.sin6.sin6_family = af;
    sa.u.sin6.sin6_port = htons(port);
    sa.u.sin6.sin6_addr = in6addr_any;
    sa.len = sizeof(sa.u.sin6);
#else
    sa.u.sin_family = af;
    sa.u.sin_port = htons(port);
    sa.u.sin_addr.s_addr = INADDR_ANY;
    sa.len = sizeof(sa.u.sin);
#endif

    if (bind(sock, &sa.u.sa, sa.len) < 0)
    {
        LOG_ERROR("Listen: af %d bind(%d):%s", af, port, strerror(ERRNO));
        return -1;
    }

    (void) listen(sock, 16);

    return (sock);
}

int main(int argc, char const *argv[])
{
    /*mc_event_t mev;
    mc_event_base_t *base = mc_base_new();
    mc_connection lc;

    int sockfd = mc_socket();
    mc_bind(sockfd);
    mc_listen(sockfd);

    mc_event_set(&(lc.read), MC_EV_READ, sockfd, handler_accept, &lc);
    mc_event_post(&(lc.read), base);
    mc_dispatch(base);*/

    /*LL_HEAD(queue);

    int i;
    for (i = 0; i < 100; i++) {
        struct vec *v = malloc(sizeof(struct vec));
        memset(v, 0, sizeof(struct vec));
        v->ptr = "demo";
        v->len = i;
        LL_ADD(&queue, &v->link);
    }

    struct llhead *lptr;
    struct vec *tmp; 
    LL_FOREACH(&queue, lptr) {
        tmp = LL_ENTRY(lptr, struct vec, link);
        LOG_DEBUG("%d, %d", tmp->ptr, tmp->len);
    }*/

    /*mc_event_base_t * base = mc_base_new();

    int i;
    for (i = 0; i < 10; i++) {
        mc_event_t * ev = malloc(sizeof(mc_event_t));

        ev->ev_fd = i;

        add_event_to_queue(ev, base->added_list);
    }

    log_printf_events(base->added_list);


    mc_base_dispose(base);*/


    /*log_printf_events((void*)base->added_list);

    printf ("\n");
    get_event_and_del((void*)base->added_list);
    get_event_and_del((void*)base->added_list);
    printf ("\n");*/

    // log_printf_events(base->added_list);

    // destroy_queue_events_safe(base->added_list);
    // destroy_queue_events_safe(base->added_list);
    //destroy_queue_events(base->added_list);
    //destroy_queue_events(base->added_list);

    mc_connection lc;
    mc_event_base_t * base = mc_base_new();

    lc.base = base; 

    int sockfd = mc_socket();



    return 0;
}

mc_event_base_t * mc_base_new(void)
{
    mc_event_base_t * base = (mc_event_base_t *)malloc(sizeof(mc_event_base_t));

    if (base == NULL)
    {
        LOG_ERROR("Init the base moudule FAIL");
        return NULL;
    }

    base->added_list = malloc(sizeof(struct llhead));
    base->active_list = malloc(sizeof(struct llhead));

    LL_INIT(base->added_list);
    LL_INIT(base->active_list);

    base->event_num = 0;
    base->event_actvie_num = 0;

    base->ev_base_stop = MC_BASE_STOP;
    base->magic = MC_BASE_MAGIC;

    gettimeofday(&base->event_time, NULL);

    mc_event_init(base);

    return base;
}

void mc_base_dispose(mc_event_base_t * base)
{
     if (base == NULL)
     {
        LOG_ERROR("Release the base moudule FAIL!");
        return ;
     }

     //删除队列中的事件
     destroy_queue_events_safe(base->added_list);
     destroy_queue_events_safe(base->active_list);

#if (HAVE_EPOLL)
     close(base->epoll_fd);
#endif

     free(base);
}

int mc_event_set(mc_event_t *ev, short revent, int fd, mc_ev_callback callback, void *args)
{
    if (ev == NULL)
    {
        LOG_ERROR("mc_event_set error!");
        return -1;
    }

#if (HAVE_EPOLL)
    unsigned int epoll_flag;
#endif

    int err;
    memset(ev, 0, sizeof(mc_event_t));

    ev->revent = revent;
    ev->ev_fd = fd;
    ev->callback = callback;

    LL_INIT(&ev->link);

    if (args == NULL)
    {
        ev->args = NULL;
    }
    else
    {
        ev->args = args;
    }

    if (ev->base == NULL)
    {
        return 0;
    }

#if (HAVE_EPOLL)
    if (revent & MC_EV_READ)
    {
        epoll_flag = EPOLLIN | EPOLLET;

        err = mc_event_mod((void*)&epoll_flag, ev);

        mc_event_mod((void*)&epoll_flag, ev);

        if (err != 0)
        {
            LOG_ERROR("mc_event_mod (MC_EVENT_READ ) in mc_event_set");
        }
    }

    if (revent & MC_EV_WRITE)
    {
        epoll_flag = EPOLLOUT | EPOLLET;

        err = mc_event_mod((void*)&epoll_flag, ev);

        if (err != 0)
        {
            LOG_ERROR("mc_event_mod (MC_EVENT_WRITE) in mc_event_set");
        }

        ev->ev_flags |= MC_EV_INITD;
    }

#endif

    return 0;
}

int mc_event_post(mc_event_t *ev, mc_event_base_t * base)
{
    if (ev == NULL || base == NULL)
    {
        LOG_ERROR("mc_event_post , the args error , please check your arguments");
        return -1;
    }

    if (base->magic != MC_BASE_MAGIC)
    {
        LOG_ERROR("The mc_event_base_t * points base non inited");
    }

    int err;
    ev->base = base;
    add_event_to_queue(ev, base->added_list);
    base->event_num++;

    err = mc_event_add(NULL, ev);

    if (err == -1)
    {
        LOG_ERROR(" mc_event_add error");
        return -1;
    }

    return 0;
}

int mc_dispatch(mc_event_base_t * base)
{
    if (base == NULL)
    {
        LOG_ERROR("base == NULL");
        return -1;
    }

    if (base->magic != MC_BASE_MAGIC)
    {
        LOG_ERROR(" reactor base noinitlized");
        return -1;
    }

    struct epoll_event *nevents = (struct epoll_event*)malloc(sizeof(struct epoll_event) * MC_EVENT_MAX);

    int  i,nevent, done = 0;
    mc_event_t *levent, *retevent;

    while (!done)
    {
         nevent = mc_event_loop(nevents, base, base->event_time);

         if (nevent == -1)
         {
            LOG_ERROR("No event check!");
            goto err1;
         }

         for (i = 0; i < nevent; i++)
         {
            if (nevents[i].events & EPOLLERR || nevents[i].events & EPOLLHUP)
            {
                levent = nevents[i].data.ptr;

                if (!(levent->ev_flags & MC_EV_INITD))
                {
                    continue;
                }

                if ((levent->ev_flags & MC_EV_ACTIVE) || (levent->ev_flags & MC_EV_ADDED))
                {
                    del_event_from_queue(levent);
                }
            }

            if (nevents[i].events & EPOLLIN)
            {
                levent = nevents[i].data.ptr;
                levent->revent = MC_EV_READ;

                add_event_to_queue(levent, (void*)base->active_list);

                levent->ev_flags |= MC_EV_ACTIVE;
                base->event_actvie_num++;
            }
            else if (nevents[i].events & EPOLLOUT)
            {
                levent = nevents[i].data.ptr;
                levent->revent = MC_EV_WRITE;

                add_event_to_queue(levent, (void*)base->active_list);

                levent->ev_flags |= MC_EV_ACTIVE;
                base->event_actvie_num++;
            }
            else
            {
                LOG_ERROR("Unkown err!");
                goto err1;
            }

        }

        for (i = 0; i < nevent; i++)
        {
            if (base->active_list->next == base->active_list->prev)
            {
                LOG_ERROR("the queue is empty now!");
                break;
            }

            retevent = get_event_and_del(base->active_list);
            base->event_actvie_num--;

            if (retevent == NULL)
            {
                LOG_ERROR("event is NULL file!");
                continue;
            }

            retevent->callback(retevent->ev_fd, retevent->revent, retevent->args);
        }
    }

        return 0;
    err1:
        return -1;
}

static void add_event_to_queue(mc_event_t *ev, void * queue)
{
    struct llhead * head = (struct llhead *) queue;

    LL_TAIL(head, &ev->link);
}

static void del_event_from_queue(mc_event_t *ev)
{
    if (ev != NULL)
    {
        LL_DEL(&ev->link);
    }
    else
    {
        LOG_ERROR("event *ev == NULL");
    }
}

static mc_event_t * get_event_and_del(void * queue)
{
    struct llhead *ptr, *head = (struct llhead *) queue;
    mc_event_t *ev;

    if (head == head->next)
    {
        LOG_ERROR("the queue is empty!");
        return NULL;
    }

    ptr = head->next;
    ev = LL_ENTRY(ptr, mc_event_t, link);

    if (ev == NULL)
    {
        return NULL;
    }

    LL_DEL(&ev->link);

    return ev;
}

static void destroy_queue_events(void * queue)
{
    struct llhead * head = (struct llhead *) queue; 
    mc_event_t *ev;

    if (head == head->next)
    {
        LOG_ERROR("the queue is empty!");
        return;
    }

    while(head != head->next)
    {
        ev = get_event_and_del(head);

        LOG_DEBUG("del event [ev_fd=%d]", ev->ev_fd);

        if (ev != NULL)
        {
            free(ev);
        }
    }

    LL_INIT(head);
}

static void destroy_queue_events_safe(void *queue)
{
    struct llhead *ptr, *tmp, *head = (struct llhead *) queue;
    mc_event_t *ev;

    if (head == head->next)
    {
        LOG_ERROR("the queue is empty!");
        return;
    }

    LL_FOREACH_SAFE(head, ptr, tmp)
    {
        ev = LL_ENTRY(ptr, mc_event_t, link);

        LOG_DEBUG("del event [ev_fd=%d]", ev->ev_fd);

        if (ev != NULL)
        {
            free(ev);
        }
    }

    LL_INIT(head);
}

static void log_printf_events(void * queue)
{
    struct llhead *ptr, *head = (struct llhead *) queue;
    mc_event_t *ev;

    if (head == head->next)
    {
        LOG_ERROR("the queue is empty!");
        return;
    }

    LL_FOREACH(head, ptr)
    {
        ev = LL_ENTRY(ptr, mc_event_t, link);
        LOG_DEBUG("event:[ev_fd=%d] [revent=%x]", ev->ev_fd, ev->revent);
    }
}

static void *mc_epoll_init(mc_event_base_t * base)
{
    if (base->magic != MC_BASE_MAGIC)
    {
        LOG_DEBUG("event base not initialize!");
        return NULL;
    }

    base->epoll_fd = epoll_create(MC_EVENT_MAX);

    return base;
}

static int mc_epoll_add(void *arg, mc_event_t *ev)
{
    if (ev->base->magic != MC_BASE_MAGIC)
    {
        LOG_DEBUG("event base not initialize!");
        return -1;
    }

    mc_event_base_t *base = ev->base;

    int err, epoll_fd = base->epoll_fd;
    struct epoll_event epoll_ev;

    epoll_ev.data.ptr = ev;
    epoll_ev.events = EPOLLIN | EPOLLET;

    if (!(ev->ev_flags & MC_EV_ADDED))
    {
        err = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, ev->ev_fd, &epoll_ev);

        if (err != 0)
        {
            LOG_DEBUG("epoll_ctl add [fd=%d] fail!", ev->ev_fd);
            return -1;
        }
        ev->ev_flags |= MC_EV_ADDED;
    }
    return 0;
}

static int mc_epoll_del(void * arg, mc_event_t *ev)
{
    if (ev->base->magic != MC_BASE_MAGIC)
    {
        LOG_ERROR("event base not initialize!");
        return -1;
    }

    mc_event_base_t *base = ev->base ;

    int err, epoll_fd = base->epoll_fd ;

    if( !(ev->ev_flags & MC_EV_INITD) )
    {
        return -1 ;
    }
    err = epoll_ctl(epoll_fd, EPOLL_CTL_DEL, ev->ev_fd, NULL);

    if( err != 0 )
    {
        LOG_DEBUG("epoll_ctl del [fd=%d] fail!", ev->ev_fd);
        return -1;
    }
    ev->ev_flags = 0x0000;

    return 0;
}

static int mc_epoll_mod(void *arg, mc_event_t *ev)
{
    if (ev->base->magic != MC_BASE_MAGIC)
    {
        LOG_DEBUG("event base not noinitlized!");
        return -1;
    }

    if (arg == NULL)
    {
        return 0;
    }

    mc_event_base_t * base = ev->base;

    int err, epoll_fd = base->epoll_fd;
    unsigned int mode;

    if (!(ev->ev_flags & MC_EV_INITD))
    {
        return -1;
    }

    struct epoll_event epoll_ev;
    epoll_ev.data.ptr = ev;

    mode = *(unsigned int *)arg;
    epoll_ev.events = mode;
    err = epoll_ctl(epoll_fd, EPOLL_CTL_MOD, ev->ev_fd, &epoll_ev);

    if (err != 0)
    {
        LOG_DEBUG("mc_epoll_del the epoll_ctl error");
        return -1;
    }

    return 1;
}

static int mc_epoll_loop(void *arg, mc_event_base_t *base, struct timeval ev_timeval)
{
    if (base == NULL)
    {
        LOG_ERROR("base == NULL");
        return -1;
    }

    if (base->magic != MC_BASE_MAGIC)
    {
        LOG_ERROR("reactor base noinitlized!");
        return -1;
    }

    int nfds;

    struct epoll_event *nevents = (struct epoll_event *) arg;
    nfds = epoll_wait(base->epoll_fd, nevents, MC_EVENT_MAX, 1);

    if (nfds <= -1)
    {
        LOG_ERROR("epoll_wait() error!");
    }

    return nfds;
}
