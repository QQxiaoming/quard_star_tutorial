#include <QTreeView>
#include <QFile>
#include <QCloseEvent>
#include <QDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QFileInfo>
#include <QDateTime>
#include <QDate>
#include <QTime>
#include <QMenu>
#include <QFileDialog>

#include "qfonticon.h"
#include "qfsviewer.h"

FSViewWindow::FSViewWindow(QWidget *parent) :
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
    QRect screen = QGuiApplication::screenAt(this->mapToGlobal(QPoint(this->width()/2,0)))->geometry();
    QRect size = this->geometry();
    this->move((screen.width() - size.width()) / 2, (screen.height() - size.height()) / 2);
}

FSViewWindow::~FSViewWindow() {
    resetView();
    delete mode;
}

FSViewWindow::mbr_t FSViewWindow::get_mbr(QString rootFSImgPath) {
    mbr_t mbr;
    QFile fs_img(rootFSImgPath);
    fs_img.open(QIODevice::ReadOnly);
    fs_img.seek(0);
    fs_img.read((char*)&mbr,sizeof(mbr));
    fs_img.close();
    return mbr;
}

void FSViewWindow::setExt4FSImgView(QString rootFSImgPath,uint64_t offset, uint64_t size) {
    setFSImgView(rootFSImgPath,offset,size,"Ext4");
}

void FSViewWindow::setFatFSImgView(QString rootFSImgPath,uint64_t offset, uint64_t size) {
    setFSImgView(rootFSImgPath,offset,size,"FatFS");
}

void FSViewWindow::setJffs2FSImgView(QString rootFSImgPath,uint64_t offset, uint64_t size) {
    setFSImgView(rootFSImgPath,offset,size,"Jffs2");
}

void FSViewWindow::resetView(void) {
    m_idle = true;
    mode->removeTree(rootIndex);
    mode->set_root_timestamp(0);
    rootIndex = mode->addTree("/", 0, 0, 0, QModelIndex());
    expand(rootIndex);
}

void FSViewWindow::expand_recursive(QString path) {
    QStringList list = path.split("/");
    QModelIndex pIndexes = rootIndex;
    foreach (QString item, list) {
        if(item == "")  {
            expand(pIndexes);
            continue;
        }
        QModelIndex cIndexes = mode->findItems(item,pIndexes);
        if(cIndexes.isValid()) {
            expand(cIndexes);
            pIndexes = cIndexes;
        } else {
            break;
        }
    }
}

void FSViewWindow::setFSImgView(QString rootFSImgPath,uint64_t offset, uint64_t size,QString type) {
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

void FSViewWindow::contextMenuEvent(QContextMenuEvent *event) {
    QModelIndex tIndex = indexAt(viewport()->mapFromGlobal(event->globalPos()));
    if (tIndex.isValid() && m_idle) {
        int type = FSViewModel::FSView_UNKNOWN;
        uint64_t size = 0;
        QString name;
        mode->info(tIndex, type, name, size);
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
                    uint64_t size = 0;
                    int type = FSViewModel::FSView_UNKNOWN;
                    mode->info(tIndex, type, name, size);
                    QString path = name;
                    std::function<QModelIndex(QModelIndex,QString &)> get_parent = [&](QModelIndex index, QString &name) -> QModelIndex {
                        if(index.isValid() && index.parent().isValid()) {
                            QString pname;
                            uint64_t size = 0;
                            int type = FSViewModel::FSView_UNKNOWN;
                            QModelIndex parent = index.parent();
                            mode->info(parent, type, pname, size);
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
                        QString filename = QFileDialog::getSaveFileName(this, tr("Save File"),QDir::homePath()+"/"+name);
                        if (filename.isEmpty())
                            return;
                        QFileInfo info(this->windowTitle());
                        int ret = -1;
                        if(fsView) ret = fsView->exportFSImg(path, filename);
                        if(ret == 0) {
                            QFileInfo savePath(filename);
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
                    uint64_t size = 0;
                    int type = FSViewModel::FSView_UNKNOWN;
                    mode->info(tIndex, type, name, size);
                    QString path = name;
                    std::function<QModelIndex(QModelIndex,QString &)> get_parent = [&](QModelIndex index, QString &name) -> QModelIndex {
                        if(index.isValid() && index.parent().isValid()) {
                            QString pname;
                            uint64_t size = 0;
                            int type = FSViewModel::FSView_UNKNOWN;
                            QModelIndex parent = index.parent();
                            mode->info(parent, type, pname, size);
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
                    #if defined(Q_OS_WIN)
                        path.replace("C:/","");
                    #endif
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
                    QFileInfo info(this->windowTitle());
                    int ret = -1;
                    if(fsView) {
                        ret = fsView->importFSImg(path, filePath);
                        if(ret == 0) {
                            resetView();
                            m_idle = false;
                            fsView->setFSImgView(rootIndex);
                            expand_recursive(path);
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

            QAction *pCreate= new QAction(tr("Create"), this);
            pCreate->setIcon(QIcon(QFontIcon::icon(QChar(0xf0f6))));
            contextMenu->addAction(pCreate);
            connect(pCreate,&QAction::triggered,this,
            [&,tIndex](void)
                {
                    int wret = QMessageBox::warning(this,"Warning","In principle, this software does not provide the function of modifying the disk image. If you use this function, please remember to back up your files, and this software does not guarantee the strict correctness of the import.\nPlease choose whether to continue.", QMessageBox::Yes, QMessageBox::No);
                    if(wret == QMessageBox::No) {
                        return;
                    }

                    QString name;
                    uint64_t size = 0;
                    int type = FSViewModel::FSView_UNKNOWN;
                    mode->info(tIndex, type, name, size);
                    QString path = name;
                    std::function<QModelIndex(QModelIndex,QString &)> get_parent = [&](QModelIndex index, QString &name) -> QModelIndex {
                        if(index.isValid() && index.parent().isValid()) {
                            QString pname;
                            uint64_t size = 0;
                            int type = FSViewModel::FSView_UNKNOWN;
                            QModelIndex parent = index.parent();
                            mode->info(parent, type, pname, size);
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
                    #if defined(Q_OS_WIN)
                        path.replace("C:/","");
                    #endif
                    }
                    bool isOK = false;
                    QString fileName = QInputDialog::getText(this, tr("Enter Dir Name"), tr("Name"), QLineEdit::Normal, "", &isOK);
                    if (fileName.isEmpty()) {
                        if(isOK) {
                            QMessageBox::critical(this, tr("Error"), tr("Can't create dir!"));
                        }
                        return;
                    }
                    path = (path=="/")?(path+fileName):(path+"/"+fileName);
                    QFileInfo info(this->windowTitle());
                    int ret = -1;
                    if(fsView) {
                        ret = fsView->createDirFSImg(path);
                        if(ret == 0) {
                            resetView();
                            m_idle = false;
                            fsView->setFSImgView(rootIndex);
                            expand_recursive(path);
                            m_idle = true;
                        }
                    }
                    if(ret == 0) {
                        QMessageBox::information(this, tr("Information"), tr("Create dir success!"));
                    } else {
                        QMessageBox::critical(this, tr("Error"), tr("Unsupported operation!"));
                    }
                }
            );
            QAction *pDelete= new QAction(tr("Delete"), this);
            pDelete->setIcon(QIcon(QFontIcon::icon(QChar(0xf014))));
            contextMenu->addAction(pDelete);
            connect(pDelete,&QAction::triggered,this,
            [&,tIndex](void)
                {
                    int wret = QMessageBox::warning(this,"Warning","In principle, this software does not provide the function of modifying the disk image. If you use this function, please remember to back up your files, and this software does not guarantee the strict correctness of the import.\nPlease choose whether to continue.", QMessageBox::Yes, QMessageBox::No);
                    if(wret == QMessageBox::No) {
                        return;
                    }

                    QString name;
                    uint64_t size = 0;
                    int type = FSViewModel::FSView_UNKNOWN;
                    mode->info(tIndex, type, name, size);
                    if(type != FSViewModel::FSView_DIR && type != FSViewModel::FSView_REG_FILE) {
                        QMessageBox::critical(this, tr("Error"), tr("Unsupported operation!"));
                        return;
                    }
                    if(type == FSViewModel::FSView_DIR && size != 0) {
                        QMessageBox::critical(this, tr("Error"), tr("Now only support delete empty dir!"));
                        return;
                    }
                    QString path = name;
                    std::function<QModelIndex(QModelIndex,QString &)> get_parent = [&](QModelIndex index, QString &name) -> QModelIndex {
                        if(index.isValid() && index.parent().isValid()) {
                            QString pname;
                            uint64_t size = 0;
                            int type = FSViewModel::FSView_UNKNOWN;
                            QModelIndex parent = index.parent();
                            mode->info(parent, type, pname, size);
                            name = (pname == "/")?(pname + name):(pname + "/" + name);
                            return get_parent(parent, name);
                        } else {
                            return QModelIndex();
                        }
                    };
                    get_parent(tIndex, path);
                    QFileInfo info(this->windowTitle());
                    int ret = -1;
                    if(fsView) {
                        if(type == FSViewModel::FSView_DIR ) {
                            ret = fsView->removeDirFSImg(path);
                        } else if(type == FSViewModel::FSView_REG_FILE) {
                            ret = fsView->removeFileFSImg(path);
                        }
                        if(ret == 0) {
                            resetView();
                            m_idle = false;
                            fsView->setFSImgView(rootIndex);
                            expand_recursive(path);
                            m_idle = true;
                        }
                    }
                    if(ret == 0) {
                        QMessageBox::information(this, tr("Information"), tr("Delete success!"));
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

void FSViewWindow::closeEvent(QCloseEvent *event) {
    if(!m_idle) {
        QMessageBox::information(this, tr("Information"), tr("Loading, please wait..."));
        event->ignore();
    } else {
        this->hide();
        event->ignore();
    }
}
