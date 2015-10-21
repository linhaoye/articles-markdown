#include <sys/socket.h> 
#include <sys/epoll.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 
#include <fcntl.h> 
#include <unistd.h> 
#include <stdio.h> 
#include <errno.h> 
#include <sys/types.h>
#include <sys/mman.h>

#define MAX_EVENTS 10000 
 
 
void Process(int listenFd);
 
struct shmstruct
{
    int count;
};
 
int main(int argc, char **argv) 
{ 
    short port = 6666; // default port 
    if(argc == 2){ 
        port = atoi(argv[1]); 
    }
 
    shm_unlink("test");
    int fd1 = shm_open("test",O_RDWR|O_CREAT|O_EXCL,666);
    struct shmstruct* ptr;
    ftruncate(fd1,sizeof(struct shmstruct));
    ptr = (struct shmstruct*)mmap(NULL,sizeof(struct shmstruct),PROT_READ|PROT_WRITE,MAP_SHARED,fd1,0);
    close(fd1);
 
    int listenFd = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(listenFd, F_SETFL, O_NONBLOCK); // 设置非阻塞方式
    sockaddr_in sin; 
    bzero(&sin, sizeof(sin)); 
    sin.sin_family = AF_INET; 
    sin.sin_addr.s_addr = INADDR_ANY; 
    sin.sin_port = htons(port); 
    bind(listenFd, (const sockaddr*)&sin, sizeof(sin)); 
    listen(listenFd, 5); 
 
    for (int i=0; i<5 ;i++)
    {
        pid_t pid = fork();
        if (pid == 0)   // 子进程
        {
            Process(listenFd); // 工作进程
            return 0;
        }
    }
    while (true)
    {
        sleep(10);
    }
 
    return 0; 
}  
 
 
void Process(int listenFd)
{
    int fd1 = shm_open("test",O_RDWR,666);
    struct shmstruct* ptr;
    ptr = (struct shmstruct*)mmap(NULL,sizeof(struct shmstruct),PROT_READ|PROT_WRITE,MAP_SHARED,fd1,0);
    close(fd1);
    struct epoll_event ev, events[MAX_EVENTS];
    //生成用于处理accept的 epoll专用的文件描述符
    int epfd = epoll_create( MAX_EVENTS );
    //设置与要处理的事件相关的文件描述符
    ev.data.fd = listenFd;
    //设置要处理的事件类型
    ev.events = EPOLLIN | EPOLLET;
    //注册epoll事件
    if ( epoll_ctl( epfd, EPOLL_CTL_ADD, listenFd, &ev ) < 0 )
    {
        printf( "worker epoll_ctl error = %s.", strerror(errno) );
        exit(1);
    }
 
    while (true)
    {
        // 等待epoll事件的发生
        int nfds = epoll_wait( epfd, events, MAX_EVENTS, -1 );
        // 处理所发生的所有事件
        for( int i=0; i<nfds; ++i ) // for循环中可以修改为线程池处理epoll事件
        {
            if( events[i].data.fd == listenFd )
            {
                socklen_t clilen;
                struct sockaddr_in clientaddr;
 
                int sockfd = accept( listenFd, (sockaddr *)&clientaddr, &clilen );
                if( sockfd < 0 )
                {
                    continue;
                }
 
                // 设置非阻塞
                if (fcntl( sockfd, F_SETFL, fcntl( sockfd, F_GETFD, 0)|O_NONBLOCK) == -1)
                {
                    continue;
                }
                //设置用于读操作的文件描述符
                ev.data.fd = sockfd;
                //设置用于注测的读操作事件
                ev.events = EPOLLIN | EPOLLET;
                //注册ev
                epoll_ctl( epfd, EPOLL_CTL_ADD, sockfd, &ev);
            }
            else if (events[i].events & EPOLLIN)
            {
                int sockfd = events[i].data.fd;
                if ( sockfd < 0 )
                {
                    continue;
                }
 
                char buf[1024] = {0};
                // 开始处理每个新连接上的数据收发
                bzero( buf, sizeof(buf) );
                int len = read( sockfd, buf, 1023 );
                if ( len < 0)
                {
                    if (errno == ECONNRESET)
                    {
                        close(sockfd);
                        events[i].data.fd = -1;
                    }
                    else
                    {
                        printf( "worker read data error = %s.", strerror(errno) );
                    }
                }
                else if ( len == 0 )
                {
                    events[i].data.fd = -1;
                }
                else
                {
                    if( send(sockfd, "asdf", 4, 0) < 0)   
                    {
                        printf("send msg error: %s(errno: %d)\n", strerror(errno), errno);
                        exit(0);
                    }
                    printf("count:%d\n",++ptr->count);
                }
            }
        }
    }
}