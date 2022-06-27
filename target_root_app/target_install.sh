SHELL_FOLDER=$(cd "$(dirname "$0")";pwd)

install_sudo()
{
    cd $SHELL_FOLDER/sudo-SUDO_1_9_7p1
    make install-binaries
}

case "$1" in
sudo)
    install_sudo
    ;;
all)
    install_sudo
    ;;
*)
    echo "Please enter the built package name or use \"all\" !"
    exit 1
	;;
esac
