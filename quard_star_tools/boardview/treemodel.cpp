#include <QDebug>
#include <QIODevice>
#include <QTreeView>
#include "treemodel.h"
#include "qfonticon.h"

class TreeItem
{
public:
	TreeItem() {}
	TreeItem(QString str, int type, TreeItem *parent) :
		m_str(str),m_type(type),m_pParent(parent)
	{
	}
	~TreeItem()
	{
		qDeleteAll(m_children) ;
	}
	void appendChild(TreeItem *pChild) { m_children.append(pChild) ; }
	void insertChild(int idx, TreeItem *pChild) { m_children.insert(idx, pChild) ; }
	void removeChild(TreeItem *p)
	{
		int index = m_children.indexOf(p) ;
		if ( index < 0 ) { return ; }
		m_children.removeAt(index) ;
		delete p ;
	}

	QString data() { return m_str ; }
	int type() { return m_type ; }
	void setData(QString str) { m_str = str ; }
	void setType(int type) { m_type = type ; }
	int childCount() { return m_children.size() ; }
	QList<TreeItem *> &children() { return m_children ; }
	TreeItem *parent() { return m_pParent ; }
	TreeItem *child(int row)
	{
		if ( row < 0 || row >= childCount() ) { return NULL ; }
		return m_children[row] ;
	}
	int row()
	{
		if ( m_pParent ) {
			return m_pParent->children().indexOf(this) ;
		}
		return 0 ;
	}

	void copy(TreeItem *p)
	{
		m_str = p->m_str ;
		for ( int i = 0 ; i < p->m_children.size() ; i ++ ) {
            insertChild(i, new TreeItem(p->m_children[i]->m_str,p->m_children[i]->m_type, this)) ;
			this->m_children[i]->copy(p->m_children[i]) ;
		}
	}

private:
	QString				m_str ;
	int				    m_type ;
	TreeItem			*m_pParent ;
	QList<TreeItem *>	m_children ;
} ;

TreeModel::TreeModel(QTreeView *parent) :
	QAbstractItemModel(parent),m_parent(parent)
{
	m_pRootItem = new TreeItem(" ", 0, NULL) ;
}

TreeModel::~TreeModel()
{
	delete m_pRootItem ;
}

QVariant TreeModel::data(const QModelIndex &index, int role) const
{
	if ( role != Qt::DisplayRole && role != Qt::EditRole  && role != Qt::DecorationRole) { return QVariant() ; }
	if ( !index.isValid() ) { return QVariant() ; }

    TreeItem *p = static_cast<TreeItem *>(index.internalPointer()) ;
    
    if (role == Qt::DecorationRole) {
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
		switch (p->type()) {
			case UNKNOWN:
				return QVariant();
			case REG_FILE:
				return QIcon(QFontIcon::icon(QChar(0xf016)));
			case DIR:
				if(m_parent->isExpanded(index))
					return QIcon(QFontIcon::icon(QChar(0xf07c)));
				else
					return QIcon(QFontIcon::icon(QChar(0xf07b)));
			case CHARDEV:
				return QIcon(QFontIcon::icon(QChar(0xf085)));
				return QIcon(QFontIcon::icon(QChar(0xf085)));
			case FIFO:
			case SOCKET:
			case SYMLINK:
				return QIcon(QFontIcon::icon(QChar(0xf0c1)));
			default:
				return QIcon(QFontIcon::icon(QChar(0xf071)));
		}
	}

	return p->data();
}

int TreeModel::rowCount(const QModelIndex &parent) const
{
	TreeItem *p = m_pRootItem ;
	if ( parent.isValid() ) {
		p = static_cast<TreeItem *>(parent.internalPointer()) ;
	}

	return p->childCount() ;
}

int TreeModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
	return 1 ;	// カラムは常に1つ
}

Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const
{
	if ( !index.isValid() ) {
		return Qt::ItemIsEnabled ;
	}
	return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable
		 | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled ;		// drag and drop処理入れる時は追加
}

bool TreeModel::setData(const QModelIndex &index, const QVariant &value, int type, int role)
{
	if ( role != Qt::DisplayRole && role != Qt::EditRole ) {
		return false ;
	}

	TreeItem *p = m_pRootItem ;
	if ( index.isValid() ) {
		p = static_cast<TreeItem *>(index.internalPointer()) ;
	}

	p->setData(value.toString()) ;
	p->setType(type) ;
	emit dataChanged(index, index);
	return true ;
}

bool TreeModel::insertRows(int row, int count, const QModelIndex &parent)
{
	beginInsertRows(parent, row, row+count-1) ;
	TreeItem *p = m_pRootItem ;
	if ( parent.isValid() ) {
		p = static_cast<TreeItem *>(parent.internalPointer()) ;
	}

    p->insertChild(row, new TreeItem(QString(), 0, p)) ;
	endInsertRows();
	return true ;
}

bool TreeModel::removeRows(int row, int count, const QModelIndex &parent)
{
	beginRemoveRows(parent, row, row+count-1) ;
	TreeItem *p = m_pRootItem ;
	if ( parent.isValid() ) {
		p = static_cast<TreeItem *>(parent.internalPointer()) ;
	}

	p->removeChild(p->child(row)) ;
	endRemoveRows();
	return true ;
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex &parent) const
{
	if ( !hasIndex(row, column, parent) ) { return QModelIndex() ; }

	TreeItem *p = m_pRootItem ;
	if ( parent.isValid() ) {
		p = static_cast<TreeItem *>(parent.internalPointer()) ;
	}

	TreeItem *pChild = p->child(row) ;
	if ( pChild ) {
		return createIndex(row, column, pChild) ;
	}
	return QModelIndex() ;
}

QModelIndex TreeModel::parent(const QModelIndex &child) const
{
	if ( !child.isValid() ) { return QModelIndex() ; }
	TreeItem *c = static_cast<TreeItem *>(child.internalPointer()) ;
	TreeItem *p = c->parent() ;
	if ( p == m_pRootItem ) { return QModelIndex() ; }
	return createIndex(p->row(), 0, p) ;
}

QVariant TreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if ( orientation == Qt::Horizontal && role == Qt::DisplayRole && section == 0 ) {
		return m_pRootItem->data() ;
	}
	return QVariant() ;
}

// drag and drop 処理 ----------------------------------------
Qt::DropActions TreeModel::supportedDropActions() const
{
	return Qt::CopyAction | Qt::MoveAction ;
}

QStringList TreeModel::mimeTypes() const
{
	QStringList types ;
	types << "application/tree.item.list" ;
	return types ;
}

QMimeData *TreeModel::mimeData(const QModelIndexList &indexes) const
{
	QMimeData *mimeData = new QMimeData() ;
	QByteArray encodeData ;

	QDataStream stream(&encodeData, QIODevice::WriteOnly) ;
	foreach ( const QModelIndex &index, indexes ) {
		if ( index.isValid() ) {
			stream << reinterpret_cast<quint64>(index.internalPointer()) ;
		}
	}
	mimeData->setData("application/tree.item.list", encodeData) ;
	return mimeData ;
}

bool TreeModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    Q_UNUSED(row);

	if ( action == Qt::IgnoreAction ) { return true ; }
	if ( !data->hasFormat("application/tree.item.list") ) { return false ; }
	if ( column > 0 ) { return false ; }

	QByteArray encodeData = data->data("application/tree.item.list") ;
	QDataStream stream(&encodeData, QIODevice::ReadOnly) ;

	while ( !stream.atEnd() ) {
		quint64 val ;
		TreeItem *p ;
		stream >> val ;
		p = reinterpret_cast<TreeItem *>(val) ;

		QString text = p->data() ;
        QModelIndex index = addTree(text, 0, parent) ;
		TreeItem *newItem = static_cast<TreeItem *>(index.internalPointer()) ;
		newItem->copy(p) ;
	}
	return true;
}
// drag and drop 処理 ここまで ----------------------------------

QModelIndex TreeModel::addTree(QString str, int type, const QModelIndex &parent)
{
	TreeItem *p = m_pRootItem ;
	if ( parent.isValid() ) {
		p = static_cast<TreeItem *>(parent.internalPointer()) ;
	}
	int row = p->childCount() ;

	insertRows(row, 1, parent) ;	// row 追加

    QModelIndex index = this->index(row, 0, parent) ;
	setData(index, str, type, Qt::DisplayRole) ;
	return index ;
}

void TreeModel::removeTree(QModelIndex &index)
{
	if ( !index.isValid() ) { return ; }

	removeRows(index.row(), 1, index.parent()) ;
}

void TreeModel::dumpTreeItems()
{
	TreeItem *p = m_pRootItem ;
	int tab = 0 ;
	qDebug() << p->data() ;
	for ( int i = 0 ; i < p->childCount() ; i ++ ) {
		_dump(p->child(i), tab + 1) ;
	}
	qDebug() << "dump end---------" ;
}

void TreeModel::_dump(TreeItem *p, int tab)
{
	QString t ;
	for ( int i = 0 ; i < tab ; i ++ ) {
		t += " " ;
	}
	t += p->data() ;
	qDebug() << t ;

	for ( int i = 0 ; i < p->childCount() ; i ++ ) {
		_dump(p->child(i), tab + 1) ;
	}
}


