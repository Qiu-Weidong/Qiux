# Qiux

#### 介绍
一个简单的操作系统


#### 安装教程

需要工具`nasm`汇编器、`gcc`编译器，`make`构建工具，以及`bximage`制作镜像文件。如果需要调试，还需要`gdb`调试工具。
```bash
# nasm安装方法
sudo apt-get install nasm
# gcc安装方法
sudo apt-get install gcc
# gdb安装方法
sudo apt-get install gdb
# make安装方法
sudo apt-get install make
# 安装g++，这是为了后面编译bochs
sudo apt-get install g++

```
`bximage`可以和`bochs`一同安装，方法如下：
1. 首先下载`bochs`源码，[bochs-2.6.11](https://sourceforge.net/projects/bochs/files/bochs/2.6.11/bochs-2.6.11.tar.gz/download)
2. 解压，使用命令`tar -zxvf bochs-2.6.11.tar.gz`，解压后会出现一个文件夹`bochs-2.6.11`
3. 进入文件夹`bochs-2.6.11`，并输入以下命令 
```bash
# --enable-gdb-stub选项是为了使用gdb来进行调试
sudo ./configure --enable-gdb-stub
sudo make
sudo make install
```
编译我们的操作系统，在Makefile的同级目录下执行`make`命令即可。
```bash
# 在Makefile的同级目录下
make
```
make成功后会得到一个Qiux.img的镜像文件，我们的操作系统就在镜像文件当中

#### 使用说明

使用`qemu`或者`bochs`运行。如果使用`bochs`，需要配置bochsrc文件，将第8行romimage配置为你的机器的BIOS-bochs-latest文件所在的目录，并将第9行vgaromimage配置为你的VGABIOS-lgpl-latest所在的目录，如果不进行调试，请将最后一行gdbstub注释掉。
```bash
# bochs,Qiux.img所在目录
bochs -f bochsrc
或
make run
# qemu，在Qiux.img所在目录
qemu-system-x86_64 -fda Qiux.img
```

#### 调试方法
使用`bochs`和`gdb`调试，请确保`bochs`使用了`gdbstub`。
首先将bochsrc最后一行的`gdbstub:enabled=1,port=1234,text_base=0,data_base=0,bss_base=0`取消注释，然后输入命令`make debug`，然后就可以使用gdb进行调试了。
```bash
# 请将gdbstub取消注释
make debug
```

#### 参与贡献

1.  Fork 本仓库
2.  新建 Feat_xxx 分支
3.  提交代码
4.  新建 Pull Request


#### 特技

1.  使用 Readme\_XXX.md 来支持不同的语言，例如 Readme\_en.md, Readme\_zh.md
2.  Gitee 官方博客 [blog.gitee.com](https://blog.gitee.com)
3.  你可以 [https://gitee.com/explore](https://gitee.com/explore) 这个地址来了解 Gitee 上的优秀开源项目
4.  [GVP](https://gitee.com/gvp) 全称是 Gitee 最有价值开源项目，是综合评定出的优秀开源项目
5.  Gitee 官方提供的使用手册 [https://gitee.com/help](https://gitee.com/help)
6.  Gitee 封面人物是一档用来展示 Gitee 会员风采的栏目 [https://gitee.com/gitee-stars/](https://gitee.com/gitee-stars/)
