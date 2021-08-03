## APP编译/配置NOTE

#### make编译

很多程序需要先在主机交叉编译，然后在目标机进行make install，因此我们先交叉编译一个make工具给目标机。配置命令如下，完成后根据需求拷贝输出到目标系统内。

```shell
./configure --host=riscv64-linux-gnu --prefix=$SHELL_FOLDER/output CXX=$CROSS_PREFIX-g++ CC=$CROSS_PREFIX-gcc 
make -j16
make install
```

make工具没有任何运行时依赖，完成。

#### ncurses编译

ncurses是一个很重要的终端绘制程序库，我们需要编译ncurses安装动态库到目标系统，首先在host系统交叉编译

```shell
./configure --host=riscv64-linux-gnu --with-shared --without-normal --without-debug CXX=$CROSS_PREFIX-g++ CC=$CROSS_PREFIX-gcc 
make -j16
make  install.libs DESTDIR=$SHELL_FOLDER/output
```

进入目标系统机中挂载ncurses目录。执行以下命令，将安装相关工具和数据到目标系统/usr/local目录中。

```
make install.progs
make install.data
```

最后拷贝输出目录下$SHELL_FOLDER/output/usr/lib/libncurses.so等库文件到目标系统。

#### bash编译

bash交叉编译比较容易，配置命令如下，完成后根据需求拷贝输出目录的/bin/bash到目标系统内。

```shell
./configure --host=riscv64 --prefix=$SHELL_FOLDER/output CCFLAGS=-I$SHELL_FOLDER/output/usr/include LDFLAGS=-L$SHELL_FOLDER/output/usr/lib CXX=$CROSS_PREFIX-g++ CC=$CROSS_PREFIX-gcc 
make -j16
make install
```

bash依赖ncurses库，因此编译时需要指向放有该库文件的输出路径。

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
./configure --host=riscv64-linux-gnu CXX=$CROSS_PREFIX-g++ CC=$CROSS_PREFIX-gcc 
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
make prefix=$SHELL_FOLDER/output CC=$CROSS_PREFIX-gcc -j16
make prefix=$SHELL_FOLDER/output CC=$CROSS_PREFIX-gcc install
```

tree没有任何运行时依赖，进入目标系统拷贝输出文件tree到/usr/bin目录就即可。

#### libevent编译

编译libevent，首先在host系统交叉编译

```shell
./configure --host=riscv64-linux-gnu --disable-openssl --disable-static --prefix=$SHELL_FOLDER/output CXX=$CROSS_PREFIX-g++ CC=$CROSS_PREFIX-gcc 
make -j16
make install
```

我们仅编译了动态库，拷贝安装到目标系统。

#### cu编译

cu是个及其简单的串口终端工具，适合嵌入式使用，但是cu在linux环境下移植不完全，我这里使用我在github找到的开源代码且自己做了些易用性的修改分叉仓库：https://github.com/QQxiaoming/cu。编译cu，首先在host系统交叉编译

```shell
make prefix=$SHELL_FOLDER/output LIBEVENTDIR=$SHELL_FOLDER/output CC=$CROSS_PREFIX-gcc -j16
make prefix=$SHELL_FOLDER/output LIBEVENTDIR=$SHELL_FOLDER/output CC=$CROSS_PREFIX-gcc install
```

动态链接了libevent了，需要上一步安装了动态库到目标系统，进入目标系统拷贝输出文件cu到/usr/bin目录就即可。

#### screen编译

编译screen，首先在host系统交叉编译

```shell
./configure --host=riscv64-linux-gnu CCFLAGS=-I$SHELL_FOLDER/output/usr/include LDFLAGS=-L$SHELL_FOLDER/output/usr/lib CXX=$CROSS_PREFIX-g++ CC=$CROSS_PREFIX-gcc 
make -j16
```

进入目标系统机中挂载screen目录。执行make install即完成，注意此时一定要部署好了bash和ncurses。

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

#### qt编译

添加编译配置文件qt-everywhere-src-5.12.11/qtbase/mkspecs/linux-riscv64-gnu-g++/qmake.conf，内容如下：

```
#
# qmake configuration for building with riscv64-unknown-linux-gnu-g++
#

MAKEFILE_GENERATOR      = UNIX
CONFIG                 += incremental
QMAKE_INCREMENTAL_STYLE = sublib

include(../common/linux.conf)
include(../common/gcc-base-unix.conf)
include(../common/g++-unix.conf)

# modifications to g++.conf
QMAKE_CC                = riscv64-unknown-linux-gnu-gcc
QMAKE_CXX               = riscv64-unknown-linux-gnu-g++
QMAKE_LINK              = riscv64-unknown-linux-gnu-g++ 
QMAKE_LINK_SHLIB        = riscv64-unknown-linux-gnu-g++
QMAKE_LIBS              = -latomic

# modifications to linux.conf
QMAKE_AR                = riscv64-unknown-linux-gnu-ar cqs
QMAKE_OBJCOPY           = riscv64-unknown-linux-gnu-objcopy
QMAKE_NM                = riscv64-unknown-linux-gnu-nm -P
QMAKE_STRIP             = riscv64-unknown-linux-gnu-strip
load(qt_config)
```

添加编译配置文件qt-everywhere-src-5.12.11/qtbase/mkspecs/linux-riscv64-gnu-g++/qplatformdefs.h，内容如下：

```c
/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the qmake spec of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "../linux-g++/qplatformdefs.h"
```

修改源码中一个缺失的头文件包含，位于qt-everywhere-src-5.12.11/qtdeclarative/src/qmldebug/qqmlprofilerevent_p.h:52行，添加如下内容：

```c
#include <limits>
```

执行以下命令进行编译

```shell
# 编译qt
export PATH=$PATH:$CROSS_COMPILE_DIR/bin
$CONFIGURE -release -opensource -ltcg -optimize-size -confirm-license -skip webengine -nomake tools -nomake tests -nomake examples -no-opengl -silent -qpa linuxfb -xplatform linux-riscv64-gnu-g++ -prefix $SHELL_FOLDER/host_output
make -j16
make install
```

#### qt配置

建议Qt库部署到/opt/下，Qt应用运行时需要一些环境变量以加载qt插件。以下是一个示例配置，qt插件的使用应以满足需求即可，太多的插件消耗大量系统内存。

```shell
export QT_HOME=/opt/Qt-5.12.11
export QT_QPA_FB_DRM=1
export QT_QPA_GENERIC_PLUGINS=evdevkeyboard
export QT_QPA_GENERIC_PLUGINS=evdevmouse
export QT_QPA_EVDEV_MOUSE_PARAMETERS=/dev/input/event0
export QT_QPA_EVDEV_KEYBOARD_PARAMETERS=/dev/input/event1
export QT_PLUGIN_PATH=$QT_HOME/plugins
```

如果Qt程序执行不正常，可以配置export QT_DEBUG_PLUGINS=1查看插件加载调试信息。

#### libmnl编译

libmnl是ethtool工具的依赖，编译动态库，完成后根据需求拷贝输出到目标系统内。

```shell
./configure --host=riscv64-linux-gnu --prefix=$SHELL_FOLDER/output CXX=$CROSS_PREFIX-g++ CC=$CROSS_PREFIX-gcc 
make -j16
make install
```

#### ethtool编译

ethtool工具编译，设置好libmnl库路径即可，完成后根据需求拷贝输出到目标系统内。

```shell
./configure --host=riscv64-linux-gnu --prefix=$SHELL_FOLDER/output MNL_CFLAGS=-I$SHELL_FOLDER/output/include MNL_LIBS="-L$SHELL_FOLDER/output/lib -lmnl" CXX=$CROSS_PREFIX-g++ CC=$CROSS_PREFIX-gcc 
make -j16
make install
```

#### openssl编译

openssl库被很多工具依赖，交叉编译如下，完成后根据需求拷贝输出到目标系统内。

```shell
./Configure linux-generic64 no-asm --prefix=$SHELL_FOLDER/output --cross-compile-prefix=$CROSS_PREFIX-
make -j16
make install_sw
```

#### iperf编译

iperf是一个网络性能测试实用工具，设置好openssl库路径即可，完成后根据需求拷贝输出到目标系统内。

```shell
./configure --host=riscv64-linux-gnu --prefix=$SHELL_FOLDER/output --with-openssl=$SHELL_FOLDER/output CXX=$CROSS_PREFIX-g++ CC=$CROSS_PREFIX-gcc 
make -j16
make install
```

#### zlib编译

zlib库被很多工具依赖，交叉编译如下，完成后根据需求拷贝输出到目标系统内。

```shell
export CC=$CROSS_PREFIX-gcc 
./configure --prefix=$SHELL_FOLDER/output
make -j16
make install
```

#### openssh编译

openssh工具提供了ssh客户端和服务端的功能，对于嵌入式开发非常有用，便于远程调试开发。交叉编译配置如下：

```shell
./configure --host=riscv64-linux-gnu --with-openssl=$SHELL_FOLDER/output --with-zlib=$SHELL_FOLDER/output CXX=$CROSS_PREFIX-g++ CC=$CROSS_PREFIX-gcc 
make -j16
```

进入目标系统机/etc/passwd文件内添加一行

```
sshd:x:74:74:Privilege-separated SSH:/var/empty/sshd:/sbin/nologin
```

挂载openssh目录。执行以下命令（安装需要strip，需要交叉编译生成目标机的本地gcc工具链，其比较麻烦，我之前手动改脚本编译出了一个，如果你没有strip可以在编译配置时传入--disable-strip省去这个步骤）：

```shell
make install
```

#### openssh配置

openssh的配置网上博客很多，这篇就不错，建议参考 https://www.cnblogs.com/schips/p/12309228.html 完成配置。当然很多路径要根据实际情况来配置，我这里给出我最终的配置。

- /usr/local/etc/sshd_config

```
#	$OpenBSD: sshd_config,v 1.103 2018/04/09 20:41:22 tj Exp $

# This is the sshd server system-wide configuration file.  See
# sshd_config(5) for more information.

# This sshd was compiled with PATH=/usr/bin:/bin:/usr/sbin:/sbin

# The strategy used for options in the default sshd_config shipped with
# OpenSSH is to specify options with their default value where
# possible, but leave them commented.  Uncommented options override the
# default value.

#Port 22
#AddressFamily any
#ListenAddress 0.0.0.0
#ListenAddress ::

HostKey /usr/local/etc/ssh_host_rsa_key
HostKey /usr/local/etc/ssh_host_ecdsa_key
HostKey /usr/local/etc/ssh_host_ed25519_key

# Ciphers and keying
#RekeyLimit default none

# Logging
#SyslogFacility AUTH
#LogLevel INFO

# Authentication:

#LoginGraceTime 2m
#PermitRootLogin prohibit-password
PermitRootLogin yes
#StrictModes yes
#MaxAuthTries 6
#MaxSessions 10

#PubkeyAuthentication yes

# The default is to check both .ssh/authorized_keys and .ssh/authorized_keys2
# but this is overridden so installations will only check .ssh/authorized_keys
AuthorizedKeysFile	.ssh/authorized_keys

#AuthorizedPrincipalsFile none

#AuthorizedKeysCommand none
#AuthorizedKeysCommandUser nobody

# For this to work you will also need host keys in /home/qqm/Downloads/quard_star_tutorial/target_root_app/output/etc/ssh_known_hosts
#HostbasedAuthentication no
# Change to yes if you don't trust ~/.ssh/known_hosts for
# HostbasedAuthentication
#IgnoreUserKnownHosts no
# Don't read the user's ~/.rhosts and ~/.shosts files
#IgnoreRhosts yes

# To disable tunneled clear text passwords, change to no here!
#PasswordAuthentication yes
#PermitEmptyPasswords no

# Change to no to disable s/key passwords
#ChallengeResponseAuthentication yes

# Kerberos options
#KerberosAuthentication no
#KerberosOrLocalPasswd yes
#KerberosTicketCleanup yes
#KerberosGetAFSToken no

# GSSAPI options
GSSAPIAuthentication no
#GSSAPICleanupCredentials yes

# Set this to 'yes' to enable PAM authentication, account processing,
# and session processing. If this is enabled, PAM authentication will
# be allowed through the ChallengeResponseAuthentication and
# PasswordAuthentication.  Depending on your PAM configuration,
# PAM authentication via ChallengeResponseAuthentication may bypass
# the setting of "PermitRootLogin without-password".
# If you just want the PAM account and session checks to run without
# PAM authentication, then enable this but set PasswordAuthentication
# and ChallengeResponseAuthentication to 'no'.
#UsePAM no

#AllowAgentForwarding yes
#AllowTcpForwarding yes
#GatewayPorts no
#X11Forwarding no
#X11DisplayOffset 10
#X11UseLocalhost yes
#PermitTTY yes
#PrintMotd yes
#PrintLastLog yes
#TCPKeepAlive yes
#PermitUserEnvironment no
#Compression delayed
#ClientAliveInterval 0
#ClientAliveCountMax 3
UseDNS no
#PidFile /var/run/sshd.pid
#MaxStartups 10:30:100
#PermitTunnel no
#ChrootDirectory none
#VersionAddendum none

# no default banner path
#Banner none

# override default of no subsystems
Subsystem	sftp	/usr/local/libexec/sftp-server

# Example of overriding settings on a per-user basis
#Match User anoncvs
#	X11Forwarding no
#	AllowTcpForwarding no
#	PermitTTY no
#	ForceCommand cvs server
```

- /etc/init.d/S90sshd.sh

```shell
#! /bin/bash
sshd=/usr/local/sbin/sshd
test -x "$sshd" || exit 0
case "$1" in
  start)
    echo -n "Starting sshd daemon"
    start-stop-daemon --start --quiet --exec $sshd  -b
    echo "."
    ;;
  stop)
    echo -n "Stopping sshd"
    start-stop-daemon --stop --quiet --exec $sshd
    echo "."
    ;;
  restart)
    echo -n "Stopping sshd"
    start-stop-daemon --stop --quiet --exec $sshd
    echo "."
    echo -n "Waiting for sshd to die off"
    for i in 1 2 3 ;
    do
        sleep 1
        echo -n "."
    done
    echo ""
    echo -n "Starting sshd daemon"
    start-stop-daemon --start --quiet --exec $sshd -b
    echo "."
    ;;
  *)
    echo "Usage: /etc/init.d/S90sshd.sh {start|stop|restart}"
    exit 1
esac
exit 0
```
