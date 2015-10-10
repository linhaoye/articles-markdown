###__VA_ARGS__用法
自定义调试信息的输出
　　调试信息的输出方法有很多种,  例如直接用printf,  或者出错时使用perror, fprintf等将信息直接打印到终端上, 在Qt上面一般使用qDebug，而守护进程则一般是使用syslog将调试信息输出到日志文件中等等...
　　使用标准的方法打印调试信息有时候不是很方便,  例如Qt编程, 在调试已有的代码时, 我想在打印调试信息的地方, 把代码位置也打印出来以方便定位错误, 或者需要在调试信息前面加一个前辍, 好方便在调试信息太多的时候可以用grep过滤一下, 仅显示本模块的调试信息, 这时就需要一个一个地修改已有的qDebug, 使其成为以下形式:
　　qDebug( "[模块名称] 调试信息  File:%s, Line:%d", __FILE__, __LINE__ );
　　这样的修改比较烦人, 而且一不小心会遗漏某个没改的...
　　为了能方便地管理调试信息的输出，一个比较简单的方法就是自已定义一个打印调试信息的宏, 然后替换原来的，废话就不多说了，直接给出一个现成的，下面是一个例子, 我用WiFi表示当前代码的模块名称，我要求在模块中的所有调试信息前面均带有[WiFi]前辍，这样我就能方便地只需使用命令行 | grep "\[WiFi\]"来过滤掉来自其它模块的调试信息了:
#define qWiFiDebug(format, ...) qDebug("[WiFi] "format" File:%s, Line:%d, Function:%s", ##__VA_ARGS__, __FILE__, __LINE__ , __FUNCTION__);
　　上面的宏是使用qDebug输出调试信息，在非Qt的程序中也可以改为printf，守护进程则可以改为syslog等等...  其中，决窍其实就是这几个宏 ##__VA_ARGS__, __FILE__, __LINE__ 和__FUNCTION__,下面介绍一下这几个宏:
　　1)  __VA_ARGS__ 是一个可变参数的宏，很少人知道这个宏，这个可变参数的宏是新的C99规范中新增的，目前似乎只有gcc支持（VC6.0的编译器不支持）。宏前面加上##的作用在于，当可变参数的个数为0时，这里的##起到把前面多余的","去掉的作用,否则会编译出错, 你可以试试。
　　2) __FILE__ 宏在预编译时会替换成当前的源文件名
　　3) __LINE__宏在预编译时会替换成当前的行号
　　4) __FUNCTION__宏在预编译时会替换成当前的函数名称
　　有了以上这几个宏，特别是有了__VA_ARGS__ ，调试信息的输出就变得灵活多了。
　　有时，我们想把调试信息输出到屏幕上，而有时则又想把它输出到一个文件中，可参考下面的例子：

```c
//debug.c
#include 
#include
//开启下面的宏表示程序运行在调试版本, 否则为发行版本, 这里假设只有调试版本才输出调试信息
#define _DEBUG
#ifdef _DEBUG
    //开启下面的宏就把调试信息输出到文件，注释即输出到终端
    #define DEBUG_TO_FILE
    #ifdef DEBUG_TO_FILE
        //调试信息输出到以下文件
        #define DEBUG_FILE "/tmp/debugmsg"
        //调试信息的缓冲长度
        #define DEBUG_BUFFER_MAX 4096
        //将调试信息输出到文件中
        #define printDebugMsg(moduleName, format, ...) {\
            char buffer[DEBUG_BUFFER_MAX+1]={0};\
            snprintf( buffer, DEBUG_BUFFER_MAX \
                    , "[%s] "format" File:%s, Line:%d\n", moduleName, ##__VA_ARGS__, __FILE__, __LINE__ );\
            FILE* fd = fopen(DEBUG_FILE, "a");\
            if ( fd != NULL ) {\
                fwrite( buffer, strlen(buffer), 1, fd );\
                fflush( fd );\
                fclose( fd );\
            }\
        }
    #else
        //将调试信息输出到终端
        #define printDebugMsg(moduleName, format, ...) \
                  printf( "[%s] "format" File:%s, Line:%d\n", moduleName, ##__VA_ARGS__, __FILE__, __LINE__ );
    #endif //end for #ifdef DEBUG_TO_FILE
#else
    //发行版本，什么也不做
    #define printDebugMsg(moduleName, format, ...)
#endif  //end for #ifdef _DEBUG
int main(int argc, char** argv)
{
    int data = 999;
    printDebugMsg( "TestProgram", "data = %d", data );
    return 0;
}
``` 
 
　　上面也说了，只有支持C99规范的gcc编译器才有__VA_ARGS__这个宏，如果不是gcc编译器，或者所用的gcc编译器版本不支持__VA_ARGS__宏怎么办？ 可参考下面的代码片段，我们换一种做法，可先将可变参数转换成字符串后，再进行输出即可:

```c
void printDebugMsg( const char* format, ...)
{
    char buffer[DEBUG_BUFFER_MAX_LENGTH + 1]={0};
    va_list arg;
    va_start (arg, format);
    vsnprintf(buffer, DEBUG_BUFFER_MAX_LENGTH, format, arg);
    va_end (arg);
    printf( "%s", buffer );
}
```