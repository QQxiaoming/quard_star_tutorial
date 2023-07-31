#ifndef BOARDWINDOW_H
#define BOARDWINDOW_H

#include <QMainWindow>
#include <QContextMenuEvent>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QPoint>
#include <QProcess>
#include <QTreeView>
#include <QMessageBox>
#include <QDateTime>
#include <QDate>
#include <QTime>

#include "jffs2extract.h"
#include "blockdev_port.h"
#include "ff.h"
#include "treemodel.h"

#include "vncwindow.h"
#include "telnetwindow.h"
#include "netselectbox.h"
#include "bootselectbox.h"

extern QString VERSION;
extern QString GIT_TAG;

namespace Ui {
class BoardWindow;
}

class FSViewWindow : public QTreeView
{
    Q_OBJECT
public:
    explicit FSViewWindow(QWidget *parent = nullptr) :
        QTreeView(parent) {
        mode = new TreeModel(this);
        setModel(mode);
        setEditTriggers(QAbstractItemView::NoEditTriggers);
        resetView();
        setWindowTitle(tr("FSView"));
        setAnimated(true);
        setColumnWidth(0,400);
        setColumnWidth(1,80);
        setColumnWidth(2,80);
        setColumnWidth(3,150);
        resize(QSize(800,600));
    }

    ~FSViewWindow() {
        resetView();
        delete mode;
    }

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

    static mbr_t get_mbr(QString rootFSImgPath) {
        mbr_t mbr;
        QFile fs_img(rootFSImgPath);
        fs_img.open(QIODevice::ReadOnly);
        fs_img.seek(0);
        fs_img.read((char*)&mbr,sizeof(mbr));
        fs_img.close();
        return mbr;
    }

    void setExt4FSImgView(QString rootFSImgPath,uint64_t offset, uint64_t size) {
        bool read_only = true;
        resetView();
        setWindowTitle(rootFSImgPath);
        QFileInfo fi(rootFSImgPath);
        mode->set_root_timestamp((uint32_t)fi.birthTime().toUTC().toSecsSinceEpoch());
        QFile fs_img(rootFSImgPath);
        fs_img.open(read_only?QIODevice::ReadOnly:QIODevice::ReadWrite);
        uint8_t *addr = fs_img.map(offset,size);
        lwext_init(addr,size);
        struct ext4_blockdev * bd = ext4_blockdev_get();
        ext4_device_register(bd, "ext4_fs");
        ext4_mount("ext4_fs", "/", read_only);
        if(!read_only) {
            ext4_recover("/");
            ext4_journal_start("/");
            ext4_cache_write_back("/", 1);
        }
        listExt4FSAll("/",rootIndex);
        if(!read_only) {
            ext4_cache_write_back("/", 0);
            ext4_journal_stop("/");
        }
        ext4_umount("/");
        ext4_device_unregister("ext4_fs");
        fs_img.unmap(addr);
        fs_img.close();
    }

    void setFatFSImgView(QString rootFSImgPath,uint64_t offset, uint64_t size) {
        resetView();
        setWindowTitle(rootFSImgPath);
        QFileInfo fi(rootFSImgPath);
        mode->set_root_timestamp((uint32_t)fi.birthTime().toUTC().toSecsSinceEpoch());
        QFile fs_img(rootFSImgPath);
        fs_img.open(QIODevice::ReadOnly);
        uint8_t *addr = fs_img.map(offset,size);
        ff_init(addr,size);
        FATFS FatFs;
        f_mount(&FatFs,"",0);
        listFatFSAll("/",rootIndex);
        f_mount(NULL,"",0);
        fs_img.unmap(addr);
        fs_img.close();
    }

    void setJffs2FSImgView(QString rootFSImgPath,uint64_t offset, uint64_t size) {
        resetView();
        setWindowTitle(rootFSImgPath);
        QFileInfo fi(rootFSImgPath);
        mode->set_root_timestamp((uint32_t)fi.birthTime().toUTC().toSecsSinceEpoch());
        QFile fs_img(rootFSImgPath);
        fs_img.open(QIODevice::ReadOnly);
        uint8_t *addr = fs_img.map(offset,size);
        jffs2_init(addr,size);
        listJffs2FSAll("/",rootIndex);
        fs_img.unmap(addr);
        fs_img.close();
    }

    void resetView(void) {
        mode->removeTree(rootIndex);
        mode->set_root_timestamp(0);
        rootIndex = mode->addTree("/", 0, 0, 0, QModelIndex());
        expand(rootIndex);
    }

protected:
    void closeEvent(QCloseEvent *event) {
        this->hide();
        event->ignore();
    }

private:
    enum fs_entity_type {
        FSView_UNKNOWN = 0,
        FSView_REG_FILE,
        FSView_DIR,
        FSView_CHARDEV,
        FSView_BLOCKDEV,
        FSView_FIFO,
        FSView_SOCKET,
        FSView_SYMLINK,
        FSView_LAST
    };
    void listExt4FSAll(QString path, QModelIndex index) {
        const ext4_direntry *de;
	    ext4_dir d;
        ext4_dir_open(&d, path.toStdString().c_str());
        de = ext4_dir_entry_next(&d);
        while (de) {
            uint32_t timestamp = 0;
            QString filename(QByteArray((const char*)de->name,de->name_length));
            if(filename == "." || filename == "..") {
                de = ext4_dir_entry_next(&d);
                continue;
            }
            QString filePath;
            if(path != "/")
                filePath = path + "/" + filename;
            else
                filePath = "/" + filename;
            ext4_ctime_get(filePath.toStdString().c_str(),&timestamp);
            switch(de->inode_type) {
                case FSView_REG_FILE:
                {
                    ext4_file fd;
	                ext4_fopen(&fd, filePath.toStdString().c_str(), "rb");
                    uint32_t size = ext4_fsize(&fd);
                    ext4_fclose(&fd);
                    mode->addTree(filename, de->inode_type, size, timestamp, index);
                    break;
                }
                case FSView_FIFO:
                case FSView_CHARDEV:
                case FSView_BLOCKDEV:
                case FSView_SYMLINK:
                case FSView_SOCKET:
                default:
                    mode->addTree(filename, de->inode_type, 0, timestamp, index);
                    break;
                case FSView_DIR:
                    QModelIndex modelIndex = mode->addTree(filename, de->inode_type, 0, timestamp, index);
                    if(path != "/")
                        listExt4FSAll(path + "/" + filename, modelIndex);
                    else
                        listExt4FSAll("/" + filename, modelIndex);
                    break;
            }

            de = ext4_dir_entry_next(&d);
        }
        ext4_dir_close(&d);
        qApp->processEvents();
    }
    
    void listFatFSAll(QString path, QModelIndex index) {
        FRESULT res; 
        DIR dir;
        FILINFO fno;
        char *fn;

        res = f_opendir(&dir, path.toStdString().c_str());
        if (res == FR_OK) {
            while (1) {
                res = f_readdir(&dir, &fno);
                if (res != FR_OK || fno.fname[0] == 0) break;
                fn = fno.fname;
                int year = ((fno.fdate & 0b1111111000000000) >> 9) + 1980;
                int month = (fno.fdate & 0b0000000111100000) >> 5;
                int day =  fno.fdate & 0b0000000000011111;
                int hour = (fno.ftime & 0b111110000000000) >> 11;
                int minute = (fno.ftime & 0b0000011111100000) >> 5;
                int second = (fno.ftime & 0b0000000000011111) * 2;
                QDateTime dt(QDate(year, month, day), QTime(hour, minute, second));

                if (fno.fattrib & AM_DIR) { 
                    QString filename(QByteArray(fn,strlen(fn)));
                    QModelIndex modelIndex = mode->addTree(filename, FSView_DIR, fno.fsize, dt.toSecsSinceEpoch(), index);
                    if(path != "/")
                        listFatFSAll(path + "/" + filename, modelIndex);
                    else
                        listFatFSAll("/" + filename, modelIndex);
                } else {
                    QString filename(QByteArray(fn,strlen(fn)));
                    uint64_t size = fno.fsize;
                    mode->addTree(filename, FSView_REG_FILE, size, dt.toSecsSinceEpoch(), index);
                }
            }
            f_closedir(&dir);
        }
        qApp->processEvents();
    }

    void listJffs2FSAll(QString path, QModelIndex index) {
        const static uint32_t dt2fsv[16] = {
            FSView_UNKNOWN,FSView_FIFO,FSView_CHARDEV,FSView_UNKNOWN,
            FSView_DIR,FSView_UNKNOWN,FSView_BLOCKDEV,FSView_UNKNOWN,
            FSView_REG_FILE,FSView_UNKNOWN,FSView_SYMLINK,FSView_UNKNOWN,
            FSView_SOCKET,FSView_UNKNOWN,FSView_UNKNOWN,FSView_UNKNOWN
        };
        struct jffs2_raw_dirent *dd;
        struct dir *d = NULL;

        uint32_t ino;
        dd = resolvepath(1, path.toStdString().c_str(), &ino);

        if (ino == 0 || (dd == NULL && ino == 0))
            qDebug("No such file or directory");
        else if ((dd == NULL && ino != 0) || (dd != NULL && dt2fsv[dd->type] == FSView_DIR)) {
            d = collectdir( ino, d);
            struct jffs2_raw_inode *ri, *tmpi;
            while (d != NULL) {
                ri = find_raw_inode( d->ino, 0);
                if (!ri) {
                    qDebug("bug: raw_inode missing!");
                    d = d->next;
                    continue;
                }
                /* Search for newer versions of the inode */
                uint32_t len = 0;
                tmpi = ri;
                while (tmpi) {
                    len = je32_to_cpu(tmpi->dsize) + je32_to_cpu(tmpi->offset);
                    tmpi = find_raw_inode(d->ino, je32_to_cpu(tmpi->version));
                }

                uint32_t timestamp = je32_to_cpu(ri->ctime);
                switch (dt2fsv[d->type]) {
                    case FSView_REG_FILE:
                    case FSView_FIFO:
                    case FSView_CHARDEV:
                    case FSView_BLOCKDEV:
                    case FSView_SYMLINK:
                    case FSView_SOCKET:
                    default:
                    {
                        QString filename(QByteArray(d->name,d->nsize));
                        mode->addTree(filename, dt2fsv[d->type], len, timestamp, index);
                        break;
                    }
                    case FSView_DIR:
                        break;
                }

                if (dt2fsv[d->type] == FSView_DIR) {
                    QString filename(QByteArray(d->name,d->nsize));
                    QModelIndex modelIndex = mode->addTree(filename, dt2fsv[d->type], 0, timestamp, index);
                    if(path != "/")
                        listJffs2FSAll(path + "/" + filename, modelIndex);
                    else
                        listJffs2FSAll("/" + filename, modelIndex);
                }

                d = d->next;
            }
            freedir(d);
        }
        qApp->processEvents();
    }

private:
    TreeModel *mode;
    QModelIndex rootIndex;
};

class BoardWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit BoardWindow(const QString &path,const QString &color = "green",
                    const bool &isDarkTheme = false,
                    QLocale::Language force_translator = QLocale::AnyLanguage,
                    QWidget *parent = nullptr);
    ~BoardWindow();
    bool powerSwitch(bool power);
    QString& getVCanName(void);
    QString& getTapName(void);
    QString& getBootCfg(void);
    bool& getUpdateCfg(void);
    int sendQemuCmd(const QString &cmd);
    static void appAbout(QWidget *parent = nullptr);

protected:
    void contextMenuEvent(QContextMenuEvent *event);
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);

private slots:
    void app_quit(void);

private:
    enum DeviceName {
        LOGO = 0,
        VGA,
        UART2,
        UART1,
        UART0,
        POWER,
        SOC,
        DDR,
        ETH,
        USB0,
        USB1,
        NAND,
        NOR,
        SD,
        SWITCH,
        JTAG,
        AUDIO,
        BOOT,

        UNKNOW = -1,
    };
    QString getOpenFileName(const QString &caption = QString(),
                            const QString &fileName = QString(),
                            const QString &filter = QString());
    void addActionGInfo(QMenu *menu,const DeviceName &title);
    void addActionOFileSystem(QMenu *menu,const DeviceName &title);
    void addActionSetting(QMenu *menu,const DeviceName &title);
    Ui::BoardWindow *ui;
    bool isMousePressed = false;
    bool powerOn = false;
    QPoint mStartPos;
    QProcess *qemuProcess;
    TelnetWindow *uartWindow[3];
    TelnetWindow *jtagWindow;
    VncWindow *lcdWindow;
    NetSelectBox *netSelect;
    BootSelectBox *bootSelect;
    FSViewWindow *fsView;
    QString envPath;
    QString skinColor;
    bool isDarkTheme;
    QString maskRomImgPath;
    QString pFlashImgPath;
    QString norFlashImgPath;
    QString nandFlashImgPath;
    QString sdImgPath;
    QString usbFlashImgPath;
    QString rootFSImgPath;
    QString tapName;
    QString vCanName;
    QString bootCfg;
    bool updateCfg;
    double scaled_value = 1.0;
    struct space
    {
        DeviceName name;
        QString drawName;
        int x1,y1,x2,y2;
        bool draw;
    }spaceList[18] = {
        {LOGO,  "",          419,558,601,588,false},
        {VGA,   tr("vga"),   315,111,649,262,false},
        {UART2, tr("uart2"), 52 ,381,107,457,false},
        {UART1, tr("uart1"), 52 ,290,107,366,false},
        {UART0, tr("uart0"), 52 ,198,107,272,false},
        {POWER, tr("power"), 127,709,222,837,false},
        {SOC,   tr("soc"),   369,377,470,479,false},
        {DDR,   tr("ddr"),   542,345,620,508,false},
        {ETH,   tr("eth"),   689,179,838,340,false},
        {USB0,  tr("usb0"),  685,367,837,504,false},
        {USB1,  tr("usb1"),  685,539,837,674,false},
        {NAND,  tr("nand"),  197,365,296,425,false},
        {NOR,   tr("nor"),   215,493,263,530,false},
        {SD,    tr("sd"),    145,114,296,263,false},
        {SWITCH,tr("switch"),247,728,385,802,false},
        {JTAG,  tr("jtag"),  52 ,499,131,671,false},
        {AUDIO, tr("audio"), 673,686,744,810,false},
        {BOOT,  tr("boot"),  409,703,650,803,false},
    };
};

#endif // BOARDWINDOW_H
