#ifndef FS_VIEW_MODEL_H
#define FS_VIEW_MODEL_H

#include <QString>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>

#include "ff_port.h"
#include "treemodel.h"

class FSViewModel
{
public:
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

    FSViewModel(TreeModel *mode, QString rootFSImgPath,uint64_t offset, uint64_t size, QWidget *parent = nullptr) {
        this->mode = mode;
        this->rootFSImgPath = rootFSImgPath;
        this->offset = offset;
        this->size = size;
        this->m_parent = parent;
    }

    virtual ~FSViewModel() {} ;

    int setFSImgView(QModelIndex &rootIndex)  {
        QFileInfo fi(rootFSImgPath);
        mode->set_root_timestamp((uint32_t)fi.birthTime().toUTC().toSecsSinceEpoch());
        QFile fs_img(rootFSImgPath);
        fs_img.open(QIODevice::ReadOnly);
        uint8_t *addr = fs_img.map(offset,size);
        if(check_fs(addr)) {
            int ret = QMessageBox::warning(m_parent,"Warning","Maybe not a correct filesystem. Do you want to force the execution, this may cause the program to crash.", QMessageBox::Yes, QMessageBox::No);
            if(ret == QMessageBox::No) {
                return -1;
            }
        }
        fs_init(addr,size,true);
        listFSAll("/",rootIndex);
        fs_deinit(addr,size,true);
        fs_img.unmap(addr);
        fs_img.close();
        return 0;
    }

    int exportFSImg(QString input, QString output) {
        QFile fs_img(rootFSImgPath);
        fs_img.open(QIODevice::ReadOnly);
        uint8_t *addr = fs_img.map(offset,size);
        fs_init(addr,size,true);
        QFile w(output);
        w.open(QIODevice::WriteOnly|QIODevice::Truncate);
        int ret = fs_write_file(input,w);
        w.close();
        fs_deinit(addr,size,true);
        fs_img.unmap(addr);
        fs_img.close();
        return ret;
    }

    int importFSImg(QString output, QString input) {
        QFile fs_img(rootFSImgPath);
        if(!fs_img.open(QIODevice::ReadWrite)) {
            qWarning() << "open fs img failed";
            return -1;
        }
        uint8_t *addr = fs_img.map(offset,size);
        fs_init(addr,size,false);
        QFile w(input);
        w.open(QIODevice::ReadOnly);
        int ret = fs_read_file(output,w);
        w.close();
        fs_deinit(addr,size,false);
        fs_img.unmap(addr);
        fs_img.close();
        return ret;
    }

    int createDirFSImg(QString dir_path) {
        QFile fs_img(rootFSImgPath);
        if(!fs_img.open(QIODevice::ReadWrite)) {
            qWarning() << "open fs img failed";
            return -1;
        }
        uint8_t *addr = fs_img.map(offset,size);
        fs_init(addr,size,false);
        int ret = fs_create_dir(dir_path);
        fs_deinit(addr,size,false);
        fs_img.unmap(addr);
        fs_img.close();
        return ret;
    }

    int removeDirFSImg(QString dir_path) {
        QFile fs_img(rootFSImgPath);
        if(!fs_img.open(QIODevice::ReadWrite)) {
            qWarning() << "open fs img failed";
            return -1;
        }
        uint8_t *addr = fs_img.map(offset,size);
        fs_init(addr,size,false);
        int ret = fs_remove_dir(dir_path);
        fs_deinit(addr,size,false);
        fs_img.unmap(addr);
        fs_img.close();
        return ret;
    }

    int removeFileFSImg(QString dir_path) {
        QFile fs_img(rootFSImgPath);
        if(!fs_img.open(QIODevice::ReadWrite)) {
            qWarning() << "open fs img failed";
            return -1;
        }
        uint8_t *addr = fs_img.map(offset,size);
        fs_init(addr,size,false);
        int ret = fs_remove_file(dir_path);
        fs_deinit(addr,size,false);
        fs_img.unmap(addr);
        fs_img.close();
        return ret;
    }

private:

    virtual int check_fs(uint8_t *addr) { Q_UNUSED(addr); return -1; }
    virtual int fs_init(uint8_t *addr, uint64_t size, bool read_only) { Q_UNUSED(addr); Q_UNUSED(size); Q_UNUSED(read_only); return -1; }
    virtual int fs_deinit(uint8_t *addr, uint64_t size, bool read_only) { Q_UNUSED(addr); Q_UNUSED(size); Q_UNUSED(read_only); return -1; }
    virtual int fs_write_file(QString input, QFile &output)  { Q_UNUSED(input); Q_UNUSED(output); return -1; }
    virtual int fs_read_file(QString output, QFile &input) { Q_UNUSED(input); Q_UNUSED(output); return -1; }
    virtual int fs_create_dir(QString path) { Q_UNUSED(path); return -1; }
    virtual int fs_remove_dir(QString path) { Q_UNUSED(path); return -1; }
    virtual int fs_remove_file(QString path) { Q_UNUSED(path); return -1; }
    virtual void listFSAll(QString path, QModelIndex index) { Q_UNUSED(path); Q_UNUSED(index); return; }

public:
    TreeModel *mode;
    QWidget *m_parent;
    QString rootFSImgPath;
    uint64_t offset;
    uint64_t size;
};

class Ext4FSViewModel : public FSViewModel
{
public:
    Ext4FSViewModel(TreeModel *mode, QString rootFSImgPath,uint64_t offset, uint64_t size, QWidget *parent = nullptr);
    ~Ext4FSViewModel() override;

private:
    int check_fs(uint8_t *addr) override;
    int fs_init(uint8_t *addr, uint64_t size, bool read_only) override;
    int fs_deinit(uint8_t *addr, uint64_t size, bool read_only) override;
    int fs_write_file(QString input, QFile &output) override;
    int fs_read_file(QString output, QFile &input) override;
    int fs_create_dir(QString path) override;
    int fs_remove_dir(QString path) override;
    int fs_remove_file(QString path) override;
    void listFSAll(QString path, QModelIndex index) override;
};

class FatFSFSViewModel : public FSViewModel
{
public:
    FatFSFSViewModel(TreeModel *mode, QString rootFSImgPath,uint64_t offset, uint64_t size, QWidget *parent = nullptr);
    ~FatFSFSViewModel() override;

private:
    int check_fs(uint8_t *addr) override;
    int fs_init(uint8_t *addr, uint64_t size, bool read_only) override;
    int fs_deinit(uint8_t *addr, uint64_t size, bool read_only) override;
    int fs_write_file(QString input, QFile &output) override;
    int fs_read_file(QString output, QFile &input) override;
    int fs_create_dir(QString path) override;
    int fs_remove_dir(QString path) override;
    int fs_remove_file(QString path) override;
    void listFSAll(QString path, QModelIndex index) override;

private:
    FATFS FatFs;
};

class Jffs2FSViewModel : public FSViewModel
{
public:
    Jffs2FSViewModel(TreeModel *mode, QString rootFSImgPath,uint64_t offset, uint64_t size, QWidget *parent = nullptr);
    ~Jffs2FSViewModel() override;

private:
    int check_fs(uint8_t *addr) override;
    int fs_init(uint8_t *addr, uint64_t size, bool read_only) override;
    int fs_deinit(uint8_t *addr, uint64_t size, bool read_only) override;
    int fs_write_file(QString input, QFile &output) override;
    int fs_read_file(QString output, QFile &input) override;
    int fs_create_dir(QString path) override;
    int fs_remove_dir(QString path) override;
    int fs_remove_file(QString path) override;
    void listFSAll(QString path, QModelIndex index) override;
};

#endif // FS_VIEW_MODEL_H
