## APP编译/配置NOTE

#### bash编译

bash交叉编译比较容易，配置命令如下，完成后根据需求拷贝输出目录的/bin/bash到目标系统内。

```shell
./configure --host=riscv64 --prefix=$SHELL_FOLDER/output CXX=$CROSS_PREFIX-g++ CC=$CROSS_PREFIX-gcc 
make -j16
make install
```

但是bash依赖ncurses库，而这个库非常重要，因此我们务必要编译安装ncurses相关文件。

#### make编译

在交叉编译ncurses遇到了非常大的阻力，目前比较稳妥的交叉编译方案是在主机交叉编译，然后在目标机进行make install，因此我们先交叉编译一个make工具给目标机。配置命令如下，完成后根据需求拷贝输出到目标系统内。

```shell
./configure --host=riscv64 --prefix=$SHELL_FOLDER/output CXX=$CROSS_PREFIX-g++ CC=$CROSS_PREFIX-gcc 
make -j16
make install
```

make工具没有任何运行时依赖，完成。

#### ncurses编译

编译ncurses，首先在host系统交叉编译

```shell
./configure --host=riscv64 --disable-stripping CXX=$CROSS_PREFIX-g++ CC=$CROSS_PREFIX-gcc 
make -j16
```

进入目标系统机中挂载ncurses目录。执行以下命令，将安装相关工具和数据到目标系统/usr/local目录中。由于我们配置是禁用了strip，如果你的目标系统存储空间有限，可以手动strip /usr/local/bin中的工具减小体积。

```
make install.progs
make install.data
```

最后拷贝交叉编译工具链目录下$CROSS_COMPILE_DIR/riscv64-buildroot-linux-gnu/sysroot/usr/lib/libncurses.so.6.1库到目标系统的/usr/local/lib/libncurses.so.6.1，并创建相关软连接/usr/local/lib/libncurses.so.6、/usr/local/lib/libncurses.so，/lib/libncurses.so.6均指向该文件。

#### bash配置

将目标系统/etc/passwd中的启动shell /bin/sh替换为/bin/bash。

添加以下内容到~/.bashrc，当然你可以根据你自己的喜好修改其中的配置。

```shell
export TERM=xterm-256color
export TERMINFO=/usr/local/share/terminfo
export PS1='[\w]\$'
```

#### sudo编译

编译sudo，首先在host系统交叉编译

```shell
./configure --host=riscv CXX=$CROSS_PREFIX-g++ CC=$CROSS_PREFIX-gcc 
make -j16
```

进入目标系统机中挂载sudo目录。执行以下命令

```shell
make install-binaries
```

添加/etc/sudoers内容如下：

```shell
#                                                                         
# This file MUST be edited with the 'visudo' command as root.             
#                                                                         
# See the sudoers man page for the details on how to write a sudoers file.
#                                                                         
 
##                                                                        
# Override built-in defaults                                              
##                                                                        
Defaults                syslog=auth,runcwd=~                              
Defaults>root           !set_logname                                      
Defaults:FULLTIMERS     !lecture,runchroot=*               
Defaults:millert        !authenticate                      
Defaults@SERVERS        log_year, logfile=/var/log/sudo.log
Defaults!PAGERS         noexec                             
                                                           
# Host alias specification                                 
                                                           
# User alias specification                                 
                                                           
# Cmnd alias specification    
                              
# User privilege specification                       
root    ALL=(ALL:ALL) ALL                                                    
                                                                             
# Members of the admin group may gain root privileges                        
%admin ALL=(ALL) ALL                                                         
                                                                             
# Allow members of group sudo to execute any command                         
%sudo   ALL=(ALL:ALL) ALL  
```
