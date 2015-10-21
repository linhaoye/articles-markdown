#include <stdio.h>
#include <time.h>
#include <sys/time.h>


#define _DEBUG

/* revent 宏定义 
 *
 * 相应的操作可以使用 | 运算来并几个需要监听的事件类型
 */ 
#define MC_EV_READ      0x0001
#define MC_EV_WRITE     0x0002
#define MC_EV_SIGNAL    0x0004
#define MC_EV_TIMEOUT   0x0008
#define MC_EV_LISTEN    0x0010

typedef void (*mc_ev_callback)(int fd, short revent, void *args);

/* 反应堆结构*/
typedef struct mc_event_base_
{
    void        *   added_list;
    void        *   active_list;
    unsigned int    event_num;
    unsigned int    event_actvie_num;

    int             epoll_fd;
    int             ev_base_stop;
    struct timeval  event_time;
} mc_event_base_t;

/*事件类型定义*/
typedef struct mc_event_
{
    struct          mc_event_ *next;
    struct          mc_event_ *prev;
    unsigned int    min_heap_index;
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

#ifdef _DEBUG
#define __QUOTE(x)      # x
#define  _QUOTE(x)      _QUOTE(x)

#define LOG_DEBUG(fmt, ...) do {                                                        \
            time_t t = time(NULL);                                                      \
            struct tm *dm = localtime(&t);                                              \
                                                                                        \
            fprintf(stdout, "[%02d:%02d:%02d] %s:[" _QUOTE(__LINE__) "]\t       %-26%:")\
                    fmt "\n", __FILE__, dm->tm_hour, dm->tm_min, dm->tm_sec, __func__,  \
                    ## __VA_ARGS__);                                                    \
            fflush(stdout);                                                             \
} while(0)

#endif

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
