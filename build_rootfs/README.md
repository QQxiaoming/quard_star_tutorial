# Build Rootfs

build_rootfs目录存放构建根文件系统映像的脚本。

## 文件说明

- build_fs.sh：挂载文件映像到/dev/loop中并拷贝文件到映像内，并写入文件到映像内部，由于挂载命令需要sudo权限，使用者务必仔细。

- build_fs_only_bootfs.sh：同build_fs.sh，但仅更新bootfs部分的文件，即Image和dtb。

- clean_gitkeep.sh：删除文件系统中的.gitkeep文件，仅保留目录。

- generate_rootfs.sh：格式化文件系统映像，会分为boofs和rootfs两个分区，分区格式为vfat和ext4，由于挂载命令需要sudo权限，使用者务必仔细。

- mount_rootfs.sh：用于调试开发使用，手动编辑根文件系统。

- sfdisk：generate_rootfs.sh所需的配置信息文件。
