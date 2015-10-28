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
    int             epoll_fd;           //epoll_create创建的epoll例程
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

//将事件加入队列
static void add_event_to_queue(mc_event_t *ev, void * queue);
//将事件从队列删除
static void del_event_from_queue(mc_event_t *ev);
//事件队列dequeue
static mc_event_t * get_event_and_del(void * queue);
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
#define mc_event_loop    mc_event_op_val.mc_dispatch;


#define mc_sock_fd          int
#define DEFAULT_NET         AF_NET
#define DEFAULT_DATA_GRAM   SOCK_STREAM
#define DEFAULT_PORT        12345
#define DEFAULT_BACKLOG     200


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


static void setreuseaddr(mc_sock_fd fd);
static int mc_socket();
static int mc_bind(mc_sock_fd listenfd);
static int mc_listen(mc_sock_fd listenfd);
static void handler_accept(int fd, short revent, void *args);
static void handler_read(int fd, short revent, void *args);
static void hnadler_write(int fd, short revent, void *args);
static void cab(int fd, short revent, void *args);

struct vec {
    int len;
    const void *ptr;
    struct llhead link;
};

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

    mc_event_base_t * base = mc_base_new();

    /*int i;
    for (i = 0; i < 10; i++) {
        mc_event_t * ev = malloc(sizeof(mc_event_t));

        ev->ev_fd = i;

        add_event_to_queue(ev, (void*)base->added_list);
    }*/

    log_printf_events((void*)base->added_list);

    printf ("\n");
    get_event_and_del((void*)base->added_list);
    get_event_and_del((void*)base->added_list);
    printf ("\n");

    log_printf_events((void*)base->added_list);

    return 0;
}

mc_event_base_t * mc_base_new(void)
{
    mc_event_base_t * base = (mc_event_base_t *)malloc(sizeof(mc_event_base_t));

    if (base == NULL) {
        LOG_ERROR("Init the base moudle FAIL");
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

    //mc_event_init(base);

    return base;
}

int mc_event_set(mc_event_t *ev, short revent, int fd, mc_ev_callback callback, void *args)
{
    if (ev == NULL) {
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
        ev->args = NULL;
    else
        ev->args = args;

    if (ev->base == NULL)
        return 0;

#if (HAVE_EPOLL)
    if (revent & MC_EV_READ) {
        epoll_flag = EPOLLIN | EPOLLET;

        err = mc_event_mod((void*)&epoll_flag, ev);

        if (err != 0)
            LOG_ERROR("mc_event_mod (MC_EVENT_READ ) in mc_event_set");
    }

    if (revent & MC_EV_WRITE) {
        epoll_flag = EPOLLOUT | EPOLLET;

        err = mc_event_mod((void*)&epoll_flag, ev);

        if (err != 0)
            LOG_ERROR("mc_event_mod (MC_EVENT_WRITE) in mc_event_set");

        ev->ev_flags |= MC_EV_INITD;
    }

#endif
    return 0;
}

int mc_event_post(mc_event_t *ev, mc_event_base_t * base)
{
    if (ev == NULL || base == NULL) {
        LOG_ERROR("mc_event_post , the args error , please check your arguments");
        return -1;
    }

    if (base->magic != MC_BASE_MAGIC) {
        LOG_ERROR("The mc_event_base_t * points base non inited");
    }

    int err;
    ev->base = base;
    LL_TAIL(base->added_list, &ev->link);
    base->event_num++;

    err = mc_event_add(NULL, ev);

    if (err == -1) {
        LOG_ERROR(" mc_event_add error");
        return -1;
    }

    return 0;
}

/*int mc_dispatch(mc_event_base_t * base)
{
    if (base == NULL) {
        LOG_ERROR("base == NULL");
        return -1;
    }

    if (base->magic != MC_BASE_MAGIC) {
        LOG_ERROR(" mc_disptahch noinitlized");
        return -1;
    }

    struct epoll_event *nevents = (struct epoll_event*)malloc(sizeof(struct epoll_event) * MC_EVENT_MAX);

    int  i,nevent, done = 0;
    mc_event_t *levent, *retevent;

    while (!done)
    {
         nevent = mc_event_loop(nevents, base, base->event_time);

         if (nevent == -1) {
            LOG_ERROR("No event check!");
            goto err1;
         }

         for (i = 0; i < nevent; i++) {
            if (nevents[i].events & EPOLLERR || nevents[i].events & EPOLLHUP) {
                levent = nevents[i].data.ptr;

                if (!(levent->ev_flags & MC_EV_INITD))
                    continue;

                if ((levent->ev_flags & MC_EV_ACTIVE) || (levent->ev_flags & MC_EV_ADDED))
                    LL_DEL(&levent->link);
            }

            if (nevents[i].events & EPOLLIN) {
                levent = nevents[i].data.ptr;
                levent->revent = MC_EV_READ;

                LL_TAIL(base->active_list, &ev->link);

                levent->ev_flags |= MC_EV_ACTIVE;
                base->event_actvie_num++;
            }
            else if (nevents[i].events & EPOLLOUT) {
                levent = nevents[i].data.ptr;
                levent->revent = MC_EV_WRITE;

                LL_TAIL(base->active_list, &levent->link);

                levent->ev_flags |= MC_EV_ACTIVE;
                base->event_actvie_num++;
            }
            else {
                LOG_ERROR("Unkown err!");
                goto err1;
            }

         }

         retevent = (mc_event_t *) (base->active_list);

         LL_FOREACH_SAFE(base->active_list, ptr, tmp) {
            retevent = LL_ENTRY(ptr, mc_event_t, link);

            LL_DEL(&retevent->link);

            retevent->ev_flags = retevent->ev_flags & (~MC_EV_ACTIVE);
            base->event_actvie_num--;

            retevent->callback(retevent->ev_fd, retevent->revent, retevent->args);
         }
    }
}*/

static void add_event_to_queue(mc_event_t *ev, void * queue)
{
    struct llhead * head = (struct llhead *) queue;

    LL_TAIL(head, &ev->link);
}

static void del_event_from_queue(mc_event_t *ev)
{
    if (ev != NULL)
        LL_DEL(&ev->link);
    else
        LOG_ERROR("event *ev == NULL");
}

static mc_event_t * get_event_and_del(void * queue)
{
    struct llhead *ptr, *head = (struct llhead *) queue;
    mc_event_t *ev;

    if (head->next == head->prev) {
        LOG_ERROR("the queue is empty!");
        return;
    }

    ptr = head->next;
    ev = LL_ENTRY(ptr, mc_event_t, link);

    if (ev == NULL)
        return NULL;

    LL_DEL(&ev->link);

    return ev;
}

static void log_printf_events(void * queue)
{
    struct llhead *ptr, *head;
    mc_event_t *ev;

    head = (struct llhead *) queue;

    if (head->next == head->prev) {
        LOG_ERROR("the queue is empty!");
        return;
    }

    LL_FOREACH(head, ptr) {
        ev = LL_ENTRY(ptr, mc_event_t, link);
        LOG_DEBUG("event:[ev_fd=%d] [revent=%x]", ev->ev_fd, ev->revent);
    }
}

static void *mc_epoll_init(mc_event_base_t *meb)
{
    if (meb->magic != MC_BASE_MAGIC) {
        LOG_DEBUG("event base not initialize!");
        return NULL;
    }
    meb->epoll_fd = epoll_create(MC_EVENT_MAX);

    return meb;
}

static int mc_epoll_add(void *arg, mc_event_t *ev)
{
    if (ev->base->magic != MC_BASE_MAGIC) {
        LOG_DEBUG("event base not initialize!");
        return -1;
    }

    mc_event_base_t *base = ev->base;

    int err, epoll_fd = base->epoll_fd;
    struct epoll_event epoll_ev;

    epoll_ev.data.ptr = ev;
    epoll_ev.events = EPOLLIN | EPOLLET;

    if (!(ev->ev_flags & MC_EV_ADDED)) {
        err = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, ev->ev_fd, &epoll_ev);

        if (err != 0) {
            LOG_DEBUG("epoll_ctl add [fd=%d] fail!", ev->ev_fd);
            return -1;
        }
        ev->ev_flags |= MC_EV_ADDED;
    }
    return 0;
}

int mc_epoll_del(void * arg, mc_event_t *ev)
{
    if (ev->base->magic != MC_BASE_MAGIC) {
        LOG_ERROR("event base not initialize!");
        return -1;
    }

    mc_event_base_t *base = ev->base ;

    int err, epoll_fd = base->epoll_fd ;

    if( !(ev->ev_flags & MC_EV_INITD) ) {
        return -1 ;
    }
    err = epoll_ctl(epoll_fd, EPOLL_CTL_DEL, ev->ev_fd, NULL);

    if( err != 0 ) {
        LOG_DEBUG("epoll_ctl del [fd=%d] fail!", ev->ev_fd);
        return -1;
    }
    ev->ev_flags = 0x0000;

    return 0;
}

int mc_epoll_mod(void *arg, mc_event_t *ev)
{
    return 1;
}

int mc_epoll_loop(void *arg, mc_event_base_t *meb, struct timeval ev_timeval)
{
    return 1;
}
