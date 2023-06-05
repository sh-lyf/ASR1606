# 一、 工具准备

1. 将tools目录下的GNUmake.exe拷贝到C:\Windows\System32和C:\Windows\SysWOW64路径下
2. 打开CMD，运行make -v，查看系统是否已经安装make工具
3. 如果没有安装make工具，拷贝tools目录下的GNUmake.exe并重命名为make.exe，将make.exe拷贝到C:\Windows\System32和C:\Windows\SysWOW64路径下

# 二、编译

1. 打开CMD命令行，进入SDK根目录
2. 输入gnumake并回车，会打印帮助信息

```cmd
D:\Workspace\code\ASR1606OPEN\cus_application\release\SIMCOM_SDK_SET>gnumake
---------------------------------------------
-
-  build method: gnumake [target]
-
-  target:[module list],[clean list]
-
-  module list:
-      A7670C_LANS
-
-  clean list:
-      clean                 [clean all modules]
-      clean_[module]        [clean a module]
-
---------------------------------------------
```

3. 根据帮助提示，输入编译指令，即可完成编译

```cmd
D:\Workspace\code\ASR1606OPEN\cus_application\release\SIMCOM_SDK_SET>gnumake A7670C_LANS
```

4. 编译完成后的目标文件在out目录下

# 三、 依赖

1. 如果cmakelist.txt的更改要引起APP的重新编译，必须要把所有用到的cmakelist.txt加入到依赖表里面去，此表保存在APP目录下的makeDepend.mak文件中
2. 表中不能有多余的声明，编译过程中会检查所有的依赖项目，如果有项目找不到，会导致编译失败
3. 如果不想添加依赖表，那么，在cmakelist.txt等makefile相关文件发生改变时，最好先执行make clean后，再进行编译
