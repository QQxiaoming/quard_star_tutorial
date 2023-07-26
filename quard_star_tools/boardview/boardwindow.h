#ifndef BOARDWINDOW_H
#define BOARDWINDOW_H

#include <QMainWindow>
#include <QContextMenuEvent>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QPoint>
#include <QProcess>
#include <QTreeView>

#include "treemodel.h"
#include "ext4_module.h"

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
        rootIndex = new QModelIndex();
    }

    ~FSViewWindow() {
        delete rootIndex;
        delete mode;
    }

    void setExt4FSImgView(QString rootFSImgPath,uint64_t offset, uint64_t size) {
        resetView();
        QFile fs_img(rootFSImgPath);
        fs_img.open(QIODevice::ReadOnly);
        ext4_init(fs_img.map(offset,size),size);
        listExt4FSAll("/",*rootIndex);
        ext4_close();
        fs_img.close();
    }

    void resetView(void) {
        mode->removeTree(*rootIndex);
    }

protected:
    void closeEvent(QCloseEvent *event) {
        this->hide();
        event->ignore();
    }

private:
    void listExt4FSAll(QString path, QModelIndex index = QModelIndex()) {
        uint64_t msize = ext4_list_contents(path.toStdString().c_str(), NULL);
        uint8_t *mdata = new uint8_t[msize];
        ext4_list_contents(path.toStdString().c_str(), mdata);
        uint8_t * p = mdata;
        while(p != (mdata + msize)) {
            enum fs_entity_type {
                UNKNOWN = 0,
                REG_FILE,
                DIR,
                CHARDEV,
                BLOCKDEV,
                FIFO,
                SOCKET,
                SYMLINK,
                LAST
            };
            struct __attribute__((packed)) ext4_ino_min_map {
                uint64_t ino;
                uint8_t type;
                uint8_t size;
                char name[1];
            };
            struct ext4_ino_min_map * mm = (struct ext4_ino_min_map *) p;
            QString filename(QByteArray(mm->name,mm->size));
            switch(mm->type) {
                case DIR :
                {
                    QModelIndex modelIndex = mode->addTree(filename, index);
                    if(path != "/")
                        listExt4FSAll(path + "/" + filename, modelIndex);
                    else
                        listExt4FSAll("/" + filename, modelIndex);
                    break;
                }
                case REG_FILE :
                default :
                {
                    mode->addTree(filename, index);
                    break;
                }
            }
            
            p += sizeof(uint64_t) + 2 + mm->size;
        }
        delete [] mdata;
    }
private:
    TreeModel *mode;
    QModelIndex *rootIndex;
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
