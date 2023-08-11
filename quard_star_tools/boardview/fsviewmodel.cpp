/*
 * This file is part of the https://github.com/QQxiaoming/QFSViewer.git
 * project.
 *
 * Copyright (C) 2023 Quard <2014500726@smail.xtu.edu.cn>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */
#include <QWidget>
#include <QApplication>


#include "jffs2extract.h"
#include "ff_port.h"
#include "lwext4_port.h"
#include "treemodel.h"
#include "fsviewmodel.h"

Ext4FSViewModel::Ext4FSViewModel(TreeModel *mode, QString rootFSImgPath,
        uint64_t offset, uint64_t size, QWidget *parent) :
    FSViewModel(mode,rootFSImgPath,offset,size,parent) {
}

Ext4FSViewModel::~Ext4FSViewModel() {
}

int Ext4FSViewModel::check_fs(uint8_t *addr) {
    if(addr[0x438] != 0x53 || addr[0x439] != 0xEF) {
        return -1;
    }
    return 0;
}

int Ext4FSViewModel::fs_init(uint8_t *addr, uint64_t size, bool read_only) {
    lwext_init(addr,size);
    struct ext4_blockdev * bd = ext4_blockdev_get();
    ext4_device_register(bd, "ext4_fs");
    ext4_mount("ext4_fs", "/", read_only);
    if(!read_only) {
        ext4_recover("/");
        ext4_journal_start("/");
        ext4_cache_write_back("/", 1);
    }
    return 0;
}

int Ext4FSViewModel::fs_deinit(uint8_t *addr, uint64_t size, bool read_only) {
    if(!read_only) {
        ext4_cache_write_back("/", 0);
        ext4_journal_stop("/");
    }
    ext4_umount("/");
    ext4_device_unregister("ext4_fs");
    Q_UNUSED(addr);
    Q_UNUSED(size);
    return 0;
}

int Ext4FSViewModel::fs_write_file(QString input, QFile &output) {
    ext4_file f;
    ext4_fopen(&f, input.toStdString().c_str(), "rb");
    uint8_t *buf = new uint8_t[4096];
    do {
        size_t byte = 0;
        ext4_fread(&f, buf, 4096, &byte);
        if(byte == 0) {
            break;
        } else {
            output.write((const char*)buf,byte);
        }
    } while(1);
    delete[] buf;
    ext4_fclose(&f);
    return 0;
}

int Ext4FSViewModel::fs_read_file(QString output, QFile &input) {
    ext4_file f;
    ext4_fopen(&f, output.toStdString().c_str(), "wb+");
    uint8_t *buf = new uint8_t[4096];
    do {
        size_t byte = 0;
        byte = input.read((char*)buf,4096);
        if(byte == 0) {
            break;
        } else {
            ext4_fwrite(&f, buf, byte, &byte);
        }
    } while(1);
    delete[] buf;
    ext4_fclose(&f);
    return 0;
}

int Ext4FSViewModel::fs_create_dir(QString path) { 
    ext4_dir_mk(path.toStdString().c_str());
    return 0;
}

int Ext4FSViewModel::fs_remove_dir(QString path) {
    int ret = ext4_dir_rm(path.toStdString().c_str());
    qDebug() << ret;
    return 0;
}

int Ext4FSViewModel::fs_remove_file(QString path) {
    ext4_fremove(path.toStdString().c_str());
    return 0;
}

void Ext4FSViewModel::listFSAll(QString path, QModelIndex index) {
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
        QString filePath = (path!="/")?(path+"/"+filename):("/"+filename);
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
                listFSAll(filePath, modelIndex);
                break;
        }

        de = ext4_dir_entry_next(&d);
    }
    ext4_dir_close(&d);
    qApp->processEvents();
}

FatFSFSViewModel::FatFSFSViewModel(TreeModel *mode, QString rootFSImgPath,
        uint64_t offset, uint64_t size, QWidget *parent) :
    FSViewModel(mode,rootFSImgPath,offset,size,parent) {
}

FatFSFSViewModel::~FatFSFSViewModel() {
}

int FatFSFSViewModel::check_fs(uint8_t *addr) {
    if(addr[0x1FE] != 0x55 || addr[0x1FF] != 0xAA) {
        return -1;
    }
    return 0;
}

int FatFSFSViewModel::fs_init(uint8_t *addr, uint64_t size, bool read_only) {
    ff_init(addr,size);
    f_mount(&FatFs,"",0);
    Q_UNUSED(read_only);
    return 0;
}

int FatFSFSViewModel::fs_deinit(uint8_t *addr, uint64_t size, bool read_only) {
    f_mount(NULL,"",0);
    Q_UNUSED(addr);
    Q_UNUSED(size);
    Q_UNUSED(read_only);
    return 0;
}

int FatFSFSViewModel::fs_write_file(QString input, QFile &output) {
    FIL f;
    f_open(&f, input.toStdString().c_str(), FA_READ);
    uint8_t *buf = new uint8_t[4096];
    do {
        UINT byte = 0;
        f_read(&f, buf, 4096, &byte);
        if(byte == 0) {
            break;
        } else {
            output.write((const char*)buf,byte);
        }
    } while(1);
    delete[] buf;
    f_close(&f);
    return 0;
}

int FatFSFSViewModel::fs_read_file(QString output, QFile &input) {
    FIL f;
    f_open(&f, output.toStdString().c_str(), FA_WRITE | FA_CREATE_ALWAYS);
    uint8_t *buf = new uint8_t[4096];
    do {
        UINT byte = 0;
        byte = input.read((char*)buf,4096);
        if(byte == 0) {
            break;
        } else {
            f_write(&f, buf, byte, &byte);
        }
    } while(1);
    delete[] buf;
    f_close(&f);
    return 0;
}

int FatFSFSViewModel::fs_create_dir(QString path) { 
    f_mkdir(path.toStdString().c_str());
    return 0;
}

int FatFSFSViewModel::fs_remove_dir(QString path) {
    f_rmdir(path.toStdString().c_str());
    return 0;
}

int FatFSFSViewModel::fs_remove_file(QString path) {
    f_unlink(path.toStdString().c_str());
    return 0;
}

void FatFSFSViewModel::listFSAll(QString path, QModelIndex index) {
    FRESULT res; 
    DIR dir;
    FILINFO fno;
    char *fn;

    res = f_opendir(&dir, path.toStdString().c_str());
    if (res == FR_OK) {
        while (1) {
            res = f_readdir(&dir, &fno);
            if (res != FR_OK || fno.fname[0] == 0) 
                break;
            fn = fno.fname;
            int year = ((fno.fdate & 0b1111111000000000) >> 9) + 1980;
            int month = (fno.fdate & 0b0000000111100000) >> 5;
            int day =  fno.fdate & 0b0000000000011111;
            int hour = (fno.ftime & 0b111110000000000) >> 11;
            int minute = (fno.ftime & 0b0000011111100000) >> 5;
            int second = (fno.ftime & 0b0000000000011111) * 2;
            QDateTime dt(QDate(year, month, day), QTime(hour, minute, second));

            QString filename(QByteArray(fn,strlen(fn)));
            if (fno.fattrib & AM_DIR) { 
                QModelIndex modelIndex = mode->addTree(filename, FSView_DIR, fno.fsize, dt.toSecsSinceEpoch(), index);
                QString filePath = (path!="/")?(path+"/"+filename):("/"+filename);
                listFSAll(filePath, modelIndex);
            } else {
                mode->addTree(filename, FSView_REG_FILE, fno.fsize, dt.toSecsSinceEpoch(), index);
            }
        }
        f_closedir(&dir);
    }
    qApp->processEvents();
}

Jffs2FSViewModel::Jffs2FSViewModel(TreeModel *mode, QString rootFSImgPath,
        uint64_t offset, uint64_t size, QWidget *parent) :
    FSViewModel(mode,rootFSImgPath,offset,size,parent) {
}

Jffs2FSViewModel::~Jffs2FSViewModel() {
}

int Jffs2FSViewModel::check_fs(uint8_t *addr) {
    if(addr[0x0] != 0x85 || addr[0x1] != 0x19) {
        return -1;
    }
    return 0;
}

int Jffs2FSViewModel::fs_init(uint8_t *addr, uint64_t size, bool read_only) {
    jffs2_init(addr,size);
    Q_UNUSED(read_only);
    return 0;
}

int Jffs2FSViewModel::fs_deinit(uint8_t *addr, uint64_t size, bool read_only) {
    Q_UNUSED(addr);
    Q_UNUSED(size);
    Q_UNUSED(read_only);
    return 0;
}

int Jffs2FSViewModel::fs_write_file(QString input, QFile &output) {
    QFileInfo input_info(input);
    QString input_path = input_info.absolutePath();
    QString input_name = input_info.fileName();
#if defined(Q_OS_WIN)
    input_path.replace("C:/","");
#endif
    struct jffs2_raw_dirent *dd;
    struct dir *d = NULL;
    uint32_t ino;
    dd = resolvepath(1, input_path.toStdString().c_str(), &ino);
    if (ino == 0 || (dd == NULL && ino == 0))
        qWarning("No such file or directory");
    else if ((dd == NULL && ino != 0) || (dd != NULL && dt2fsv[dd->type] == FSView_DIR)) {
        d = collectdir( ino, d);
        struct jffs2_raw_inode *ri, *tmpi;
        while (d != NULL) {
            ri = find_raw_inode( d->ino, 0);
            if (!ri) {
                qWarning("bug: raw_inode missing!");
                d = d->next;
                continue;
            }

            tmpi = ri;
            while (tmpi) {
                tmpi = find_raw_inode(d->ino, je32_to_cpu(tmpi->version));
            }
            QString filename(QByteArray(d->name,d->nsize));
            if(dt2fsv[d->type] == FSView_REG_FILE) {
                if(filename == input_name) {
                    while(ri) {
                        size_t sz;
                        size_t len = qMax(je32_to_cpu(ri->isize),(je32_to_cpu(ri->offset) + je32_to_cpu(ri->dsize)));
                        uint8_t *buf = new uint8_t[len];
                        putblock((char *)buf, len, &sz, ri);
                        output.write((const char*)buf,sz);
                        delete[] buf;
                        ri = find_raw_inode(d->ino, je32_to_cpu(ri->version));
                    }
                }
            }
            d = d->next;
        }
        freedir(d);
    }
    return 0;
}

int Jffs2FSViewModel::fs_read_file(QString output, QFile &input) {
    QFileInfo output_info(output);
    QString output_path = output_info.absolutePath();
    QString output_name = output_info.fileName();
#if defined(Q_OS_WIN)
    output_path.replace("C:/","");
#endif

    struct jffs2_raw_dirent *dd;
    uint32_t ino;
    dd = resolvepath(1, output_path.toStdString().c_str(), &ino);
    if (ino == 0 || (dd == NULL && ino == 0))
        qWarning("No such file or directory");
    else if ((dd == NULL && ino != 0) || (dd != NULL && dt2fsv[dd->type] == FSView_DIR)) {
        uint32_t free_ino;
        uint64_t free_offset;
        find_free(&free_ino, &free_offset);
        //TODOL: write file 
        uint8_t *buf = new uint8_t[input.size()];
        input.read((char*)buf,input.size());
        QDateTime dt = QDateTime::currentDateTime();
        write_file(output_name.toStdString().c_str(),buf,input.size(), ino, free_ino, dt.toSecsSinceEpoch(), free_offset, 0, 4);
        delete[] buf;
    }
    return 0;
}

int Jffs2FSViewModel::fs_create_dir(QString path) { 
    QFileInfo output_info(path);
    QString output_path = output_info.absolutePath();
    QString output_name = output_info.fileName();
#if defined(Q_OS_WIN)
    output_path.replace("C:/","");
#endif

    struct jffs2_raw_dirent *dd;
    uint32_t ino;
    dd = resolvepath(1, output_path.toStdString().c_str(), &ino);
    if (ino == 0 || (dd == NULL && ino == 0))
        qWarning("No such file or directory");
    else if ((dd == NULL && ino != 0) || (dd != NULL && dt2fsv[dd->type] == FSView_DIR)) {
        uint32_t free_ino;
        uint64_t free_offset;
        find_free(&free_ino, &free_offset);
        QDateTime dt = QDateTime::currentDateTime();
        write_dir(output_name.toStdString().c_str(), ino, free_ino, dt.toSecsSinceEpoch(), free_offset, 0, 4);
    }
    return 0;
}

int Jffs2FSViewModel::fs_remove_dir(QString path) {
    struct jffs2_raw_dirent *dd;
    uint32_t ino;
    int ret = -1;
    dd = resolvepath(1, path.toStdString().c_str(), &ino);
    if (ino == 0 || (dd == NULL && ino == 0))
        qWarning("No such file or directory");
    else if ((dd == NULL && ino != 0) || (dd != NULL && dt2fsv[dd->type] == FSView_DIR)) {
        ret = deletenode(ino);
    }
    return ret;
}

int Jffs2FSViewModel::fs_remove_file(QString path) {
    QFileInfo output_info(path);
    QString output_path = output_info.absolutePath();
    QString output_name = output_info.fileName();
#if defined(Q_OS_WIN)
    output_path.replace("C:/","");
#endif

    struct jffs2_raw_dirent *dd;
    struct dir *d = NULL;
    uint32_t ino;
    int ret = -1;
    dd = resolvepath(1, output_path.toStdString().c_str(), &ino);
    if (ino == 0 || (dd == NULL && ino == 0))
        qWarning("No such file or directory");
    else if ((dd == NULL && ino != 0) || (dd != NULL && dt2fsv[dd->type] == FSView_DIR)) {
        d = collectdir( ino, d);
        struct jffs2_raw_inode *ri, *tmpi;
        while (d != NULL) {
            ri = find_raw_inode( d->ino, 0);
            if (!ri) {
                qWarning("bug: raw_inode missing!");
                d = d->next;
                continue;
            }

            tmpi = ri;
            while (tmpi) {
                tmpi = find_raw_inode(d->ino, je32_to_cpu(tmpi->version));
            }
            QString filename(QByteArray(d->name,d->nsize));
            if(dt2fsv[d->type] == FSView_REG_FILE) {
                if(filename == output_name) {
                    ret = deletenode(d->ino);
                    break;
                }
            }
            d = d->next;
        }
        freedir(d);
    }
    return ret;
}

void Jffs2FSViewModel::listFSAll(QString path, QModelIndex index) {
    struct jffs2_raw_dirent *dd;
    struct dir *d = NULL;

    uint32_t ino;
    dd = resolvepath(1, path.toStdString().c_str(), &ino);

    if (ino == 0 || (dd == NULL && ino == 0))
        qWarning("No such file or directory");
    else if ((dd == NULL && ino != 0) || (dd != NULL && dt2fsv[dd->type] == FSView_DIR)) {
        d = collectdir( ino, d);
        struct jffs2_raw_inode *ri, *tmpi;
        while (d != NULL) {
            ri = find_raw_inode( d->ino, 0);
            if (!ri) {
                qWarning("bug: raw_inode missing!");
                d = d->next;
                continue;
            }

            uint32_t len = 0;
            tmpi = ri;
            while (tmpi) {
                len = je32_to_cpu(tmpi->dsize) + je32_to_cpu(tmpi->offset);
                tmpi = find_raw_inode(d->ino, je32_to_cpu(tmpi->version));
            }
            uint32_t timestamp = je32_to_cpu(ri->ctime);
            QString filename(QByteArray(d->name,d->nsize));
            switch (dt2fsv[d->type]) {
                case FSView_REG_FILE:
                case FSView_FIFO:
                case FSView_CHARDEV:
                case FSView_BLOCKDEV:
                case FSView_SYMLINK:
                case FSView_SOCKET:
                default:
                {
                    mode->addTree(filename, dt2fsv[d->type], len, timestamp, index);
                    break;
                }
                case FSView_DIR:
                {
                    QString filePath = (path!="/")?(path+"/"+filename):("/"+filename);
                    QModelIndex modelIndex = mode->addTree(filename, dt2fsv[d->type], 0, timestamp, index);
                    listFSAll(filePath, modelIndex);
                    break;
                }
            }

            d = d->next;
        }
        freedir(d);
    }
    qApp->processEvents();
}
