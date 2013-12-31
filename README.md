姓名：张成

学号：U201012954

班级：电信1006班

项目名称：自己动手写嵌入式Web Server

这是2013年嵌入式Linux课程设计项目“自己动手写嵌入式Web Server”提交的源码和可运行程序

www/ 该目录下是webserver静态网页的目录

etc/ 该目录是webserver配置文件所在目录

log/ 该目录是webserver产生的日志文件所在目录

zlib/ 这是zlib库，由于本webserver用zlib来对静态网页进行gzip压缩，在编译时和运行时均要zlib的存在


tiny_webserver.c 程序代码文件

Makefile 程序的makefile，直接在本目录运行make命令即可获得可执行命令

tiny_websrv 可执行二进制文件 32bit平台下elf文件


运行说明：

在根目录下运行make命令，生成可执行文件

运行./tiny_websrv即可

然后在浏览器地址栏输入127.0.0.1：8080即可看到效果

想看到具体浏览器和webserver的交互，最好在firefox下使用firebug插件来测试


Author：张成 Ricky Zhang

Email：whrickyzc@gmail.com

Tel：13163259987


@2013年6月13日
