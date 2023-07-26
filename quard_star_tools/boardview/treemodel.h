#ifndef TREEMODEL_H
#define TREEMODEL_H

#include <QAbstractItemModel>
#include <QStringList>
#include <QMimeData>
#include <QTreeView>

class TreeItem ;

class TreeModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit TreeModel(QTreeView *parent = 0);
	~TreeModel() ;

	QVariant data(const QModelIndex &index, int role) const ;
	int rowCount(const QModelIndex &parent) const ;
	int columnCount(const QModelIndex &parent) const ;
	Qt::ItemFlags flags(const QModelIndex &index) const ;
	bool setData(const QModelIndex &index, const QVariant &value, int type, int role) ;
	bool insertRows(int row, int count, const QModelIndex &parent) ;
	bool removeRows(int row, int count, const QModelIndex &parent) ;
	QModelIndex index(int row, int column, const QModelIndex &parent) const ;
	QModelIndex parent(const QModelIndex &child) const ;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const ;

	// drag and drop 追加
	Qt::DropActions supportedDropActions() const ;
	QStringList mimeTypes() const ;
	QMimeData *mimeData(const QModelIndexList &indexes) const ;
	bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) ;
	// -----------------

    QModelIndex addTree(QString str, int type, const QModelIndex &parent) ;
    void removeTree(QModelIndex &parent) ;

	void dumpTreeItems() ;

private:
	void _dump(TreeItem *p, int tab) ;

signals:

public slots:

private:
	TreeItem	*m_pRootItem ;
    QTreeView   *m_parent;
};

#endif // TREEMODEL_H
