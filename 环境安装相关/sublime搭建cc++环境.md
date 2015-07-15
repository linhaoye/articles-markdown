##WINDWOS下, 在sublime text 2中搭建c/c++环境
###一.安装sublime
```
到官网上下载sublime text 2软件, 按提示安装.
```

###二.c/c++环境

1.安装c/c++环境
```
因为我们用到的C/C++编译器使用的是gcc/g++, 所以需要下载安装MinGW, 这里为了方便, 我下载一个dev c++(里面也包含MinGw环境).
```

2.设置环境变量
```
1、在PATH里加入C:\MinGW\bin，记得，如果里面还有其他的变量，记得要加个分号啊，分号得在英文输入模式下输入的。
2、新建LIBRARY_PATH变量，如果有的话，在值中加入C:\MinGW\lib，这是标准库的位置。
3、新建C_INCLUDEDE_PATH变量，值设为C:\MinGW\include。
4、面就是要判断一下我们的MinGW环境是否安装成功。直接运行cmd命令行，输入g++ -v。
```

###三.设置sublime text

Windows下，要在Sublime Text 2中实现编译、运行C/C++代码，需要修改或新建一个c/c++编译配置, 这里我们以c为例。

具体是：
Sublime Text 2中Tools –> Build System –> New Build System…

```javascript
{
     "cmd": ["g++", "${file}", "-o", "${file_path}/${file_base_name}"],
     "file_regex": "^(..[^:]*):([0-9]+):?([0-9]+)?:? (.*)$",
     "working_dir": "${file_path}",
     "selector": "source.c, source.c++",
     "shell": true,
     "variants":
     [
          {
               "name": "Run",
               "cmd": [ "start", "${file_path}/${file_base_name}.exe"]
          }
     ]
}
```

在Windows中，该文件被保存在Sublime Text 2目录下的Data\Packages\User中。(下面的代码不能直接运行)

```javascript
{
     "cmd": ["g++", "${file}", "-o", "${file_path}/${file_base_name}"], // For GCC On Windows and Linux
     //"cmd": ["CL", "/Fo${file_base_name}", "/O2", "${file}"],     // For CL on Windows Only
     "file_regex": "^(..[^:]*):([0-9]+):?([0-9]+)?:? (.*)$",
     "working_dir": "${file_path}",
     "selector": "source.c, source.c++",     
     "variants":
     [
          {
               "name": "Run",
               //"cmd": ["bash", "-c", "g++ '${file}' -o '${file_path}/${file_base_name}' && '${file_path}/${file_base_name}'"]  // Linux Only
               "cmd": ["CMD", "/U", "/C", "g++ ${file} -o ${file_base_name} && ${file_base_name}"]  // For GCC On Windows Only
               //"cmd": ["CMD", "/U", "/C", "CL /Fo${file_base_name} /O2 ${file} && ${file_base_name}"]   // For CL On Windows Only
          }
     ]
}
```

###四.编译一个c代码示例

搭建好C/C++编译环境后，Sublime Text 2中编译运行C/C++代码了。
ctrl+B构建，ctrl+shift+B运行。

```c

main(void)
{
	
}
```
