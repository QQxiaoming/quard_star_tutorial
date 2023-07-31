

HEADERS += \
    $$PWD/include/misc/queue.h \
    $$PWD/include/misc/tree.h \
    $$PWD/include/ext4_balloc.h \
    $$PWD/include/ext4_bcache.h \
    $$PWD/include/ext4_bitmap.h \
    $$PWD/include/ext4_blockdev.h \
    $$PWD/include/ext4_block_group.h \
    $$PWD/include/ext4_config.h \
    $$PWD/include/ext4_crc32.h \
    $$PWD/include/ext4_debug.h \
    $$PWD/include/ext4_dir.h \
    $$PWD/include/ext4_dir_idx.h \
    $$PWD/include/ext4_errno.h \
    $$PWD/include/ext4_extent.h \
    $$PWD/include/ext4_fs.h \
    $$PWD/include/ext4.h \
    $$PWD/include/ext4_hash.h \
    $$PWD/include/ext4_ialloc.h \
    $$PWD/include/ext4_inode.h \
    $$PWD/include/ext4_journal.h \
    $$PWD/include/ext4_mbr.h \
    $$PWD/include/ext4_misc.h \
    $$PWD/include/ext4_mkfs.h \
    $$PWD/include/ext4_oflags.h \
    $$PWD/include/ext4_super.h \
    $$PWD/include/ext4_trans.h \
    $$PWD/include/ext4_types.h \
    $$PWD/include/ext4_xattr.h \
    $$PWD/blockdev_port.c

SOURCES += \
    $$PWD/src/ext4_balloc.c \
    $$PWD/src/ext4_bcache.c \
    $$PWD/src/ext4_bitmap.c \
    $$PWD/src/ext4_blockdev.c \
    $$PWD/src/ext4_block_group.c \
    $$PWD/src/ext4.c \
    $$PWD/src/ext4_crc32.c \
    $$PWD/src/ext4_debug.c \
    $$PWD/src/ext4_dir.c \
    $$PWD/src/ext4_dir_idx.c \
    $$PWD/src/ext4_extent.c \
    $$PWD/src/ext4_fs.c \
    $$PWD/src/ext4_hash.c \
    $$PWD/src/ext4_ialloc.c \
    $$PWD/src/ext4_inode.c \
    $$PWD/src/ext4_journal.c \
    $$PWD/src/ext4_mbr.c \
    $$PWD/src/ext4_mkfs.c \
    $$PWD/src/ext4_super.c \
    $$PWD/src/ext4_trans.c \
    $$PWD/src/ext4_xattr.c \
    $$PWD/blockdev_port.c

INCLUDEPATH += $$PWD
INCLUDEPATH += $$PWD/include
INCLUDEPATH += $$PWD/include/misc
