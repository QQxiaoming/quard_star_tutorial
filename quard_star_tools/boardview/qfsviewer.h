#ifndef QFSVIEWER_H
#define QFSVIEWER_H

#include <QTreeView>
#include <QCloseEvent>

#include "treemodel.h"
#include "fsviewmodel.h"

class FSViewWindow : public QTreeView
{
    Q_OBJECT
public:
    explicit FSViewWindow(QWidget *parent = nullptr);
    ~FSViewWindow();

    enum {
        LIBPARTMBR_STATUS_NON_BOOTABLE     = 0x00,
        LIBPARTMBR_STATUS_NON_BOOTABLE_LBA = 0x01,
        LIBPARTMBR_STATUS_BOOTABLE         = 0x80,
        LIBPARTMBR_STATUS_BOOTABLE_LBA     = 0x81,
    };

    enum {
        LIBPARTMBR_TYPE_EMPTY             = 0x00,	    // empty
        LIBPARTMBR_TYPE_FAT12_32MB        = 0x01,	    // FAT12 within the first 32MB, or anywhere in logical drivve
        LIBPARTMBR_TYPE_XENIX_ROOT        = 0x02,	    // XENIX root
        LIBPARTMBR_TYPE_XENIX_USR         = 0x03,	    // XENIX usr
        LIBPARTMBR_TYPE_FAT16_32MB        = 0x04,	    // FAT16 within the first 32MB, less than 65536 sectors, or anywhere in logical drive
        LIBPARTMBR_TYPE_EXTENDED_CHS      = 0x05,	    // extended partition (CHS mapping)
        LIBPARTMBR_TYPE_FAT16B_8GB        = 0x06,	    // FAT16B (CHS) within the first 8GB, 65536 or more sectors, or FAT12/FAT16 outside first 32MB, or in type 0x05 extended part
        LIBPARTMBR_TYPE_NTFS_HPFS         = 0x07,	    // OS/2 IFS/HPFS, Windows NT NTFS, Windows CE exFAT
        LIBPARTMBR_TYPE_LOGSECT_FAT16     = 0x08,	    // Logically sectored FAT12/FAT16 (larger sectors to overcome limits)

        LIBPARTMBR_TYPE_FAT32_CHS         = 0x0B,	    // FAT32 (CHS)
        LIBPARTMBR_TYPE_FAT32_LBA         = 0x0C,	    // FAT32 (LBA)

        LIBPARTMBR_TYPE_FAT16B_LBA        = 0x0E,		// FAT16B (LBA)
        LIBPARTMBR_TYPE_EXTENDED_LBA      = 0x0F,		// extended partition (LBA mapping)

        LIBPARTMBR_TYPE_FAT12_32MB_HIDDEN = 0x11,	    // hidden version of type 0x01

        LIBPARTMBR_TYPE_FAT16_32MB_HIDDEN = 0x14,		// hidden version of type 0x04

        LIBPARTMBR_TYPE_FAT16B_8GB_HIDDEN = 0x16,	    // hidden version of type 0x06
        LIBPARTMBR_TYPE_NTFS_HPFS_HIDDEN  = 0x17,		// hidden version of type 0x07

        LIBPARTMBR_TYPE_FAT32_CHS_HIDDEN  = 0x1B,		// hidden version of type 0x0B
        LIBPARTMBR_TYPE_FAT32_LBA_HIDDEN  = 0x1C,		// hidden version of type 0x0C

        LIBPARTMBR_TYPE_FAT16B_LBA_HIDDEN = 0x1E,		// hidden version of type 0x0E

        LIBPARTMBR_TYPE_LINUX_SWAP        = 0x82,		// Linux swap
        LIBPARTMBR_TYPE_LINUX_NATIVE      = 0x83,		// Linux native partition

        LIBPARTMBR_TYPE_GPT               = 0xEE,	    // GPT protective partition

        LIBPARTMBR_TYPE_LINUX_RAID        = 0xFD,		// Linux RAID partition
    };

    const static uint16_t MBR_COPY_PROTECTED = 0x5A5A;
    const static uint16_t MBR_BOOT_SIGNATURE = 0xAA55;

    #pragma pack(1)
    typedef struct {
        uint8_t   status;
        struct {
            uint8_t   h;
            uint16_t  cs;
        } start_chs;
        uint8_t   type;
        struct {
            uint8_t   h;
            uint16_t  cs;
        } end_chs;
        uint32_t  starting_lba;
        uint32_t  number_of_sectors;
    } partition_t;
    typedef struct {
        uint8_t              bootstrap_code[440];
        uint32_t             disk_signiture;
        uint16_t             copy_protected;
        partition_t          partition_table[4];
        uint16_t             boot_signature;
    } mbr_t;
    #pragma pack()

    static mbr_t get_mbr(QString rootFSImgPath);
    void setExt4FSImgView(QString rootFSImgPath,uint64_t offset, uint64_t size);
    void setFatFSImgView(QString rootFSImgPath,uint64_t offset, uint64_t size);
    void setJffs2FSImgView(QString rootFSImgPath,uint64_t offset, uint64_t size);

private:
    void resetView(void);
    void expand_recursive(QString path);
    void setFSImgView(QString rootFSImgPath,uint64_t offset, uint64_t size,QString type);

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private:
    TreeModel *mode;
    FSViewModel *fsView = nullptr;
    bool m_idle;
    QMenu *contextMenu = nullptr;
    QModelIndex rootIndex;
};

#endif // QFSVIEWER_H

