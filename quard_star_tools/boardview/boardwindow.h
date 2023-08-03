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
#include <QMenu>
#include <QFileDialog>

#include "qfonticon.h"

#include "fsviewmodel.h"
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
        setWindowModality(Qt::ApplicationModal);
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
        setFSImgView(rootFSImgPath,offset,size,"Ext4");
    }

    void setFatFSImgView(QString rootFSImgPath,uint64_t offset, uint64_t size) {
        setFSImgView(rootFSImgPath,offset,size,"FatFS");
    }

    void setJffs2FSImgView(QString rootFSImgPath,uint64_t offset, uint64_t size) {
        setFSImgView(rootFSImgPath,offset,size,"Jffs2");
    }

private:
    void resetView(void) {
        m_idle = true;
        mode->removeTree(rootIndex);
        mode->set_root_timestamp(0);
        rootIndex = mode->addTree("/", 0, 0, 0, QModelIndex());
        expand(rootIndex);
    }

    void setFSImgView(QString rootFSImgPath,uint64_t offset, uint64_t size,QString type) {
        resetView();
        m_idle = false;
        setWindowTitle(rootFSImgPath);
        if(fsView) delete fsView;
        if(type == "Ext4") {
            fsView = new Ext4FSViewModel(mode,rootFSImgPath,offset,size,this);
        } else if(type == "FatFS") {
            fsView = new FatFSFSViewModel(mode,rootFSImgPath,offset,size,this);
        } else if(type == "Jffs2") {
            fsView = new Jffs2FSViewModel(mode,rootFSImgPath,offset,size,this);
        }
        fsView->setFSImgView(rootIndex);
        m_idle = true;
    }

protected:
    void contextMenuEvent(QContextMenuEvent *event) override {
        QModelIndex tIndex = indexAt(viewport()->mapFromGlobal(event->globalPos()));
        if (tIndex.isValid() && m_idle) {
            int type = FSViewModel::FSView_UNKNOWN;
            QString name;
            mode->info(tIndex, type, name);
            if((type == FSViewModel::FSView_REG_FILE) || (type == FSViewModel::FSView_DIR)) {
                //TODO: why this way crash?
                //QMenu *contextMenu = new QMenu(this); 
                //contextMenu->setAttribute(Qt::WA_DeleteOnClose); 
                // Now we renew contextMenu, because use Qt::WA_DeleteOnClose can't work
                if(contextMenu) delete contextMenu;
                contextMenu = new QMenu(this); 

                QAction *pExport= new QAction(tr("Export"), this);
                pExport->setIcon(QIcon(QFontIcon::icon(QChar(0xf019))));
                contextMenu->addAction(pExport);
                connect(pExport,&QAction::triggered,this,
                    [&,tIndex](void)
                    {
                        QString name;
                        int type = FSViewModel::FSView_UNKNOWN;
                        mode->info(tIndex, type, name);
                        QString path = name;
                        std::function<QModelIndex(QModelIndex,QString &)> get_parent = [&](QModelIndex index, QString &name) -> QModelIndex {
                            if(index.isValid() && index.parent().isValid()) {
                                QString pname;
                                int type = FSViewModel::FSView_UNKNOWN;
                                QModelIndex parent = index.parent();
                                mode->info(parent, type, pname);
                                name = (pname == "/")?(pname + name):(pname + "/" + name);
                                return get_parent(parent, name);
                            } else {
                                return QModelIndex();
                            }
                        };
                        get_parent(tIndex, path);
                        if(type == FSViewModel::FSView_DIR) {
                            QMessageBox::critical(this, tr("Error"), tr("Exporting dirs is not currently supported!"));
                            return;
                        } else if(type == FSViewModel::FSView_REG_FILE) {
                            QString filename = QFileDialog::getSaveFileName(this, tr("Save File"), name);
                            if (filename.isEmpty())
                                return;
                            int ret = -1;
                            if(fsView) ret = fsView->exportFSImg(path, filename);
                            if(ret == 0) {
                                QMessageBox::information(this, tr("Information"), tr("Export file success!"));
                            } else {
                                QMessageBox::critical(this, tr("Error"), tr("Can't export file!"));
                            }
                        } else {
                            QMessageBox::critical(this, tr("Error"), tr("Can't export file!"));
                            return;
                        }
                    }
                );

                QAction *pImport= new QAction(tr("Import"), this);
                pImport->setIcon(QIcon(QFontIcon::icon(QChar(0xf093))));
                contextMenu->addAction(pImport);
                connect(pImport,&QAction::triggered,this,
                    [&,tIndex](void)
                    {
                        int wret = QMessageBox::warning(this,"Warning","In principle, this software does not provide the function of modifying the disk image. If you use this function, please remember to back up your files, and this software does not guarantee the strict correctness of the import.\nPlease choose whether to continue.", QMessageBox::Yes, QMessageBox::No);
                        if(wret == QMessageBox::No) {
                            return;
                        }

                        QString name;
                        int type = FSViewModel::FSView_UNKNOWN;
                        mode->info(tIndex, type, name);
                        QString path = name;
                        std::function<QModelIndex(QModelIndex,QString &)> get_parent = [&](QModelIndex index, QString &name) -> QModelIndex {
                            if(index.isValid() && index.parent().isValid()) {
                                QString pname;
                                int type = FSViewModel::FSView_UNKNOWN;
                                QModelIndex parent = index.parent();
                                mode->info(parent, type, pname);
                                name = (pname == "/")?(pname + name):(pname + "/" + name);
                                return get_parent(parent, name);
                            } else {
                                return QModelIndex();
                            }
                        };
                        get_parent(tIndex, path);
                        if(type == FSViewModel::FSView_REG_FILE) {
                            QFileInfo input_info(path);
                            path = input_info.absolutePath();
                        }
                        QString filePath = QFileDialog::getOpenFileName(this, tr("Open File"), QDir::homePath());
                        if (filePath.isEmpty())
                            return;
                        QFileInfo input(filePath);
                        if(!input.isFile()) {
                            QMessageBox::critical(this, tr("Error"), tr("Can't import file!"));
                            return;
                        }
                        path = (path=="/")?(path+input.fileName()):(path+"/"+input.fileName());
                        int ret = -1;
                        if(fsView) {
                            ret = fsView->importFSImg(path, filePath);
                            if(ret == 0) {
                                resetView();
                                m_idle = false;
                                fsView->setFSImgView(rootIndex);
                                m_idle = true;
                            }
                        }
                        if(ret == 0) {
                            QMessageBox::information(this, tr("Information"), tr("Import file success!"));
                        } else {
                            QMessageBox::critical(this, tr("Error"), tr("Unsupported operation!"));
                        }
                    }
                );
                if(!contextMenu->isEmpty()) {
                    contextMenu->move(cursor().pos());
                    contextMenu->show();
                }
            }
        }
        event->accept();
    }

    void closeEvent(QCloseEvent *event) override {
        if(!m_idle) {
            QMessageBox::information(this, tr("Information"), tr("Loading, please wait..."));
            event->ignore();
        } else {
            this->hide();
            event->ignore();
        }
    }

private:
    TreeModel *mode;
    FSViewModel *fsView = nullptr;
    bool m_idle;
    QMenu *contextMenu = nullptr;
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
    QMenu *contextMenu = nullptr;
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
