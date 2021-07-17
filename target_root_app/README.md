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

#### tree编译

编译tree，首先在host系统交叉编译

```shell
cd $SHELL_FOLDER/tree-1.8.0
make prefix=$SHELL_FOLDER/output CC=$CROSS_PREFIX-gcc -j16
make prefix=$SHELL_FOLDER/output CC=$CROSS_PREFIX-gcc install
```

tree没有任何运行时依赖，进入目标系统拷贝输出文件tree到/usr/bin目录就即可。

#### libevent编译

编译libevent，首先在host系统交叉编译

```shell
cd $SHELL_FOLDER/libevent-2.1.12-stable
$CONFIGURE --host=riscv --disable-openssl --prefix=$SHELL_FOLDER/output CXX=$CROSS_PREFIX-g++ CC=$CROSS_PREFIX-gcc 
make -j16
make install
```

我们仅编译了静态库，用来编译一些需要依赖这个库的程序，不需要安装到目标系统。

#### cu编译

cu是个及其简单的串口终端工具，适合嵌入式使用，但是cu在linux环境下移植不完全，我这里使用我在github找到的开源代码且自己做了些易用性的修改分叉仓库：https://github.com/QQxiaoming/cu。编译cu，首先在host系统交叉编译

```shell
cd $SHELL_FOLDER/cu
make prefix=$SHELL_FOLDER/output LIBEVENTDIR=$SHELL_FOLDER/output CC=$CROSS_PREFIX-gcc -j16
make prefix=$SHELL_FOLDER/output LIBEVENTDIR=$SHELL_FOLDER/output CC=$CROSS_PREFIX-gcc install
```

静态链接了libevent了，这样没有任何动态依赖，进入目标系统拷贝输出文件cu到/usr/bin目录就即可。

#### screen编译

编译screen，首先在host系统交叉编译

```shell
cd $SHELL_FOLDER/screen-4.8.0
$CONFIGURE --host=riscv64 CXX=$CROSS_PREFIX-g++ CC=$CROSS_PREFIX-gcc 
make -j16
```

进入目标系统机中挂载screen目录。执行make install即完成，主要此时一定要部署好了bash和ncurses。

#### screen配置

为了能更好的使用screen，给大家推荐一套screen配置，编辑/etc/screenrc文件内容如下：

```
#
# Example of a user's .screenrc file
#

# This is how one can set a reattach password:
# password ODSJQf.4IJN7E    # "1234"

# no annoying audible bell, please
vbell on

# detach on hangup
autodetach on

# don't display the copyright page
startup_message off

# emulate .logout message
pow_detach_msg "Screen session of \$LOGNAME \$:cr:\$:nl:ended."

# advertise hardstatus support to $TERMCAP
# termcapinfo  * '' 'hs:ts=\E_:fs=\E\\:ds=\E_\E\\'

# make the shell in every window a login shell
#shell -$SHELL

# autoaka testing
# shellaka '> |tcsh'
# shellaka '$ |sh'

# set every new windows hardstatus line to somenthing descriptive
# defhstatus "screen: ^En (^Et)"

defscrollback 1000

# don't kill window after the process died
# zombie "^["

# enable support for the "alternate screen" capability in all windows
# altscreen on

################
#
# xterm tweaks
#

#xterm understands both im/ic and doesn't have a status line.
#Note: Do not specify im and ic in the real termcap/info file as
#some programs (e.g. vi) will not work anymore.
termcap  xterm hs@:cs=\E[%i%d;%dr:im=\E[4h:ei=\E[4l
terminfo xterm hs@:cs=\E[%i%p1%d;%p2%dr:im=\E[4h:ei=\E[4l

#80/132 column switching must be enabled for ^AW to work
#change init sequence to not switch width
termcapinfo  xterm Z0=\E[?3h:Z1=\E[?3l:is=\E[r\E[m\E[2J\E[H\E[?7h\E[?1;4;6l

# Make the output buffer large for (fast) xterms.
#termcapinfo xterm* OL=10000
termcapinfo xterm* OL=100

# tell screen that xterm can switch to dark background and has function
# keys.
termcapinfo xterm 'VR=\E[?5h:VN=\E[?5l'
termcapinfo xterm 'k1=\E[11~:k2=\E[12~:k3=\E[13~:k4=\E[14~'
termcapinfo xterm 'kh=\EOH:kI=\E[2~:kD=\E[3~:kH=\EOF:kP=\E[5~:kN=\E[6~'

# special xterm hardstatus: use the window title.
termcapinfo xterm 'hs:ts=\E]2;:fs=\007:ds=\E]2;screen\007'

#terminfo xterm 'vb=\E[?5h$<200/>\E[?5l'
termcapinfo xterm 'vi=\E[?25l:ve=\E[34h\E[?25h:vs=\E[34l'

# emulate part of the 'K' charset
termcapinfo   xterm 'XC=K%,%\E(B,[\304,\\\\\326,]\334,{\344,|\366,}\374,~\337'

# xterm-52 tweaks:
# - uses background color for delete operations
termcapinfo xterm* be

################
#
# wyse terminals
#

#wyse-75-42 must have flow control (xo = "terminal uses xon/xoff")
#essential to have it here, as this is a slow terminal.
termcapinfo wy75-42 xo:hs@

# New termcap sequences for cursor application mode.
termcapinfo wy* CS=\E[?1h:CE=\E[?1l:vi=\E[?25l:ve=\E[?25h:VR=\E[?5h:VN=\E[?5l:cb=\E[1K:CD=\E[1J

################
#
# other terminals
#

# make hp700 termcap/info better
termcapinfo  hp700 'Z0=\E[?3h:Z1=\E[?3l:hs:ts=\E[62"p\E[0$~\E[2$~\E[1$}:fs=\E[0}\E[61"p:ds=\E[62"p\E[1$~\E[61"p:ic@'

# Extend the vt100 desciption by some sequences.
termcap  vt100* ms:AL=\E[%dL:DL=\E[%dM:UP=\E[%dA:DO=\E[%dB:LE=\E[%dD:RI=\E[%dC
terminfo vt100* ms:AL=\E[%p1%dL:DL=\E[%p1%dM:UP=\E[%p1%dA:DO=\E[%p1%dB:LE=\E[%p1%dD:RI=\E[%p1%dC
termcapinfo linux C8
# old rxvt versions also need this
# termcapinfo rxvt C8


################
#
# keybindings
#

#remove some stupid / dangerous key bindings
bind k
bind ^k
bind .
bind ^\
bind \\
bind ^h
bind h
#make them better
bind 'K' kill
bind 'I' login on
bind 'O' login off
bind '}' history

# Yet another hack:
# Prepend/append register [/] to the paste if ^a^] is pressed.
# This lets me have autoindent mode in vi.
register [ "\033:se noai\015a"
register ] "\033:se ai\015a"
bind ^] paste [.]

################
#
# default windows
#

# screen -t local 0
# screen -t mail 1 mutt
# screen -t 40 2 rlogin server

# caption always "%3n %t%? @%u%?%? [%h]%?%=%c"
# hardstatus alwaysignore
# hardstatus alwayslastline "%Lw"

# bind = resize =
# bind + resize +1
# bind - resize -1
# bind _ resize max
#
# defnonblock 1
# blankerprg rain -d 100
# idle 30 blanker
```

另外可选编辑~/.screenrc文件内容（个人喜好配置在启动screen时创建三个终端窗口）如下：

```
# basic settings... self explainatory
deflogin on
autodetach on
startup_message off

# the dash makes it a login shell
defshell bash
activity ""
bell_msg ""
vbell off
vbell_msg ""
defscrollback 2048
nonblock on
defutf8 on
defbce on
defflow off
msgwait 1
altscreen on

# only Shift+k to kill - prevent accidents
bind k
bind ^k
bind K kill

escape \\\

term screen-256color
terminfo rxvt-unicode ti@:te@:
 
hardstatus alwayslastline "%{=}%-w%{+b w}%n %t%{-b w}%+w %=%c"
 
#let pgup/pgdn scroll under urxvt (see .Xdefaults)
bindkey "^[[5;2~" eval "copy" "stuff ^u"
bindkey -m "^[[5;2~" stuff ^u
bindkey -m "^[[6;2~" stuff ^d

screen -t window0 bash
screen -t window1 bash
select window0
split -v
focus right
select window1
screen -t window2 bash
split
focus down
select window2
focus up
select window1
focus left
```

最后要正确的显示screen必须要将终端的字符长款正确配置，命令如下：

```shell
stty rows $ROWS cols $COLS
```

那么终端的长宽如何确定呢，因为我们使用qemu指定终端大小，因此我这里使用qemu的fw_cfg将参数传入内核，如果你有别的更好的办法也可以告诉我，如果你是实际板卡也可以根据具体情况配置参数。如下命令加入.bashrc的脚本，就能正确从我们的qemu启动脚本中解析型如“qemu_vc=:vn:$COLSx$ROWS:”的参数到目标机器了。

```shell
QEMU_VPARAM="$(cat /sys/firmware/qemu_fw_cfg/by_name/opt/qemu_cmdline/raw | sed 's/\(.*\)qemu_vc=:\(.*\):\(.*\)/\2/g')"
ROWS="$(echo $QEMU_VPARAM | sed 's/\(.*\)vn:\(.*\)x\(.*\)/\2/g')"
COLS="$(echo $QEMU_VPARAM | sed 's/\(.*\)vn:\(.*\)x\(.*\)/\3/g')"
stty rows $ROWS cols $COLS
```
