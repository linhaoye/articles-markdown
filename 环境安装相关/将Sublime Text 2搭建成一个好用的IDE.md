#将Sublime Text 2搭建成一个好用的IDE

　　说起编辑器，可能大部分人要推荐的是Vim和Emacs，本人用过Vim，功能确实强大，但是不是很习惯，之前一直有朋友推荐SUblime Text 2这款编辑器，然后这段时间就试了一下，就深深地喜欢上这款编辑器了，对于类似的编辑器，我用过notepad2、notepad++、Editplus、UltraEdit、Vim、TextPad，都没有觉得哪一款编辑器用得非常上手，直到遇到Sublime Text 2，之前写Python脚本时，也一直在苦苦寻找一款好用的IDE，用过WingIDE、Ulipad、Pycharm、Eclipse+Pydev、PyScrypter，没有哪款非常中意的，直到遇到了Sublime Text 2，今天就来讲一下如何将Sublime Text 2打造成一款好用的IDE，虽然它只是一款编辑器，但是它有丰富的扩展插件，足以让我们把它变成好用的IDE。

###一.下载和安装

　　Sublime Text2是一款开源的软件，不需要注册即可使用（虽然没有注册会有弹窗，但是基本不影响使用）。

　　下载地址：http://www.sublimetext.com/，请自行根据系统版本进行下载。下载好之后直接安装即可。

　　默认的sublime的配置文件是在C盘目录下的，如果有朋友觉得放C盘不习惯，那么在安装好sublime之后，不要直接运行sublime，在地址栏里输入%appdata%，然后删除该目录下的sublime text 2文件夹，然后在sublime的安装目录下建立一个名为Data的文件夹，再运行sublime，以后关于sublime的所有配置文件都在Data目录下了。

　　如果有朋友不习惯其英文界面，可以使用汉化包进行汉化，我把汉化包上传到博客空间了，需要的朋友可以进行下载。

　　汉化包地址：http://files.cnblogs.com/dolphin0520/sublime%E6%B1%89%E5%8C%96.rar

　　汉化方法：

　　1.运行sublime text 2；

　　2.选择“preferences”—>“Browse packages”；

　　3.找到文件夹“Default”，将解压得到的文件复制到“Default”文件夹下覆盖即可。

　　下面是汉化后的运行界面：



　　关掉sublime的更新提示：

　　sublime默认的情况会有更新提示弹出框，可以把通过设置关闭更新提示，方法：

　　选择 “preferences”—>“Browse packages”，找打Default文件夹下的Preferences.sublime-settings，在最下面加一行（注意要先在前一行最后面加一个逗号）：

　　"update_check":false

　　保存退出重新启动sublime即可。

###二.一些常用的设置和快捷键

　　1.字体、主题风格等设置

　　当需要更改主题时，直接可以通过“preferences”—>“颜色主题”来设置：

　　

　　主界面上只能改变字体的大小：

　　

　　若需要改变字体和字体大小，可以先”preferences“—>”Browse Packages“，找到”Default“文件夹，然后找到Preferences.sublime-settings这个文件，用Sublime Text 2打开这个文件，这个文件保存了一些常用的设置，



　　比如字体、主题风格、是否显示行号、智能提示延迟时间等，可以根据自己的需要自行设置。

　　2.打开（关闭）侧边栏、右边缩略图等常用面板

　　默认情况下Sublime Text 2是没有打开侧边栏文件浏览器的，可以通过”查看“—>”侧边栏“—>”隐藏侧边栏“来打开和关闭侧边栏

　　默认情况下Sublime Text 2右边是有文件的缩略图的，可以通过”查看“—>”隐藏迷你地图“来打开和关闭缩略图。

　　3.快捷键寻找文件和已定义的函数

　　在Sublime Text 2中可以非常快速地切换到想找的文件，只需要通过”Ctrl+P“打开切换面板即可。

　　

　　然后输入想找的文件名称就可以快速找切换到该文件了。如果想要找函数，可以通过输入”@+函数名“可以快速切换到定义该函数的文件。

###三.一些必备的插件。

　　下面来介绍一些Sublime Text 2中必备的常用插件。

　　Sublime Text 2安装的插件和所有预置的插件全部在Packages文件下，可以直接通过”preferences“—>”Browse Pakcages“来访问。

　　Sublime Text 2安装插件有两种方法：

　　1）离线安装，先下载好安装包，解压之后放到Packages文件夹下，重启Sublime即可。

　　2）在线安装，在线安装之前，需要安装”Packages Control“这个包管理插件，安装方法是：

　　　　选择”查看“—>”显示控制台“，然后在下面弹出的框中输入：
```python
import urllib2,os;pf='Package Control.sublime-package';ipp=sublime.installed_packages_path();os.makedirs(ipp) if not os.path.exists(ipp) else None;open(os.path.join(ipp,pf),'wb').write(urllib2.urlopen('http://sublime.wbond.net/'+pf.replace(' ','%20')).read())
```
　　　　然后回车确认，安装完毕之后重启sublime，如果发现在Perferences中看到package control这一项，则安装成功。

　　然后就可以通过”Ctrl+Shift+P“打开命令面板，输入”install“命令，就可以看到安装包列表了。

　　

　　下面推荐几款必备的常用插件：

　　1.Tag插件

　　Tag插件可以为web开发者提供html和css标签，很方便快捷，对于web前端设计者非常实用。

　　2.Prefixr插件

　　为css3提供一些前缀

　　3.Terminal插件

　　Terminal插件可以允许在Sublime Text2中打开cmd命令窗口，很实用的一个插件，安装好该插件好，打开cmd命令窗口的快捷键是

Ctrl+Shift+T。

　　4.SublimeTmpl插件

　　这个插件允许用户定义文件的模板，比如在写一个html文件时，老是重复文件头的一些引入信息很繁琐，可以定义一个模板直接生成必须的信息，具体的SublimeTmpl插件用法请自行百度。

　　5.SideBarEnhancements插件

　　一个增强侧边栏文件夹浏览功能的插件，比较不错。

　　6.DocBlockr插件

　　用来生成注释块的插件，安装好之后直接输入"/*"，然后再按回车键，即可生成代码注释块。

　　7.SublimeCodeIntel插件

　　智能提示插件，这个插件的智能提示功能非常强大，可以自定义提示的内容库，我的Python智能提示设置（配置文件路径为packages\SublimeCodeIntel-master\.codeintel\config）为：
```javascript
{
    "Python": {
        "python":'D:/Program Files/Python26/python.exe',
        "pythonExtraPaths": ['D:/Program Files/Python26','D:/Program Files/Python26/DLLs','D:/Program Files/Python26/Lib','D:/Program Files/Python26/Lib/plat-win','D:/Program Files/Python26/Lib/lib-tk','D:/Program Files/Python26/Lib/site-packages']
    }
}
```
　　其中“pythonExtraPaths”就是需要智能提示所需要用到的内容库。

　　8.emmet

	前段神器, 减少大量的工作量, 使用方法可以参考Emmet：HTML/CSS代码快速编写神器或者官方文档

　　9.AndyJS2插件

　　一款针对Javsscript和jquery智能提示的插件。

　　10.jquery插件

　　jquery提示库。

　　11.Ctags插件

　　该插件可以实现快速定位到函数定义的地方。

	12.alignment

	这个忘了干嘛的了, 好像是控制所有类型文本的缩进

	13.converttoUTF8

	编辑的所有文件都使用UTF-8编码

	14.markdownediting或者markdownPerview

	这个是写Markdown必备的。可以在包管理器中安装。装完之后，写作Markdown时（右下角显示语法为Markdown），可以按ctrl+b，直接就会生成HTML，并在浏览器中显示。

	16.jsformat

	JavaScript代码格式化

	17.SyntaxChecker
	语法检测, 支持 Ruby , Perl , PHP , XML(需安装其环境)

　　18.为了避免打开含中文字符的文件出现乱码，需要先安装GBK Encoding Support这个插件，再安装ConvertToUTF8插件即可。