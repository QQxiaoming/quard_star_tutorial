/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the test suite of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/
#include <QStringList>
#include <QtQml/qqmlextensionplugin.h>
#include <QtQml/qqml.h>
#include <private/qqmlengine_p.h>
#include <QDebug>
#include <private/qv8engine_p.h>
#include <QtSql/qsqldatabase.h>
#include <QtSql/qsqlquery.h>
#include <QtSql/qsqlerror.h>
#include <QtSql/qsqlrecord.h>
#include <QtSql/qsqlfield.h>
#include <QtCore/qstandardpaths.h>
#include <QtCore/qstack.h>
#include <QtCore/qcryptographichash.h>
#include <QtCore/qsettings.h>
#include <QtCore/qdir.h>
#include <private/qv4sqlerrors_p.h>
#include <private/qv4engine_p.h>
#include <private/qv4object_p.h>
#include <private/qv4functionobject_p.h>
#include <private/qv4objectproto_p.h>
#include <private/qv4scopedvalue_p.h>
#include <private/qv4jscall_p.h>
#include <private/qv4objectiterator_p.h>

QT_BEGIN_NAMESPACE

#define V4THROW_SQL(error, desc) { \
    QV4::ScopedString v(scope, scope.engine->newString(desc)); \
    QV4::ScopedObject ex(scope, scope.engine->newErrorObject(v)); \
    ex->put(QV4::ScopedString(scope, scope.engine->newIdentifier(QStringLiteral("code"))).getPointer(), QV4::ScopedValue(scope, Value::fromInt32(error))); \
    scope.engine->throwError(ex); \
    RETURN_UNDEFINED(); \
}

#define V4THROW_SQL2(error, desc) { \
    QV4::ScopedString v(scope, scope.engine->newString(desc)); \
    QV4::ScopedObject ex(scope, scope.engine->newErrorObject(v)); \
    ex->put(QV4::ScopedString(scope, scope.engine->newIdentifier(QStringLiteral("code"))).getPointer(), QV4::ScopedValue(scope, Value::fromInt32(error))); \
    args->setReturnValue(scope.engine->throwError(ex)); \
    return; \
}

#define V4THROW_REFERENCE(string) { \
    QV4::ScopedString v(scope, scope.engine->newString(string)); \
    scope.engine->throwReferenceError(v); \
    RETURN_UNDEFINED(); \
}


class QQmlSqlDatabaseData : public QV8Engine::Deletable
{
public:
    QQmlSqlDatabaseData(QV4::ExecutionEngine *engine);
    ~QQmlSqlDatabaseData() override;

    QV4::PersistentValue databaseProto;
    QV4::PersistentValue queryProto;
    QV4::PersistentValue rowsProto;
};

V4_DEFINE_EXTENSION(QQmlSqlDatabaseData, databaseData)

namespace QV4 {

namespace Heap {
    struct QQmlSqlDatabaseWrapper : public Object {
        enum Type { Database, Query, Rows };
        void init()
        {
            Object::init();
            type = Database;
            database = new QSqlDatabase;
            version = new QString;
            sqlQuery = new QSqlQuery;
        }

        void destroy() {
            delete database;
            delete version;
            delete sqlQuery;
            Object::destroy();
        }

        Type type;
        QSqlDatabase *database;

        QString *version; // type == Database

        bool inTransaction; // type == Query
        bool readonly;   // type == Query

        QSqlQuery *sqlQuery; // type == Rows
        bool forwardOnly; // type == Rows
    };
}

class QQmlSqlDatabaseWrapper : public Object
{
public:
    V4_OBJECT2(QQmlSqlDatabaseWrapper, Object)
    V4_NEEDS_DESTROY

    static Heap::QQmlSqlDatabaseWrapper *create(QV4::ExecutionEngine *engine)
    {
        return engine->memoryManager->allocate<QQmlSqlDatabaseWrapper>();
    }

    ~QQmlSqlDatabaseWrapper() {
    }

    static ReturnedValue virtualGet(const Managed *m, PropertyKey id, const Value *receiver, bool *hasProperty);
};

}

using namespace QV4;

DEFINE_OBJECT_VTABLE(QV4::QQmlSqlDatabaseWrapper);



static ReturnedValue qmlsqldatabase_version(const FunctionObject *b, const Value *thisObject, const QV4::Value *, int)
{
    Scope scope(b);
    QV4::Scoped<QQmlSqlDatabaseWrapper> r(scope, thisObject->as<QQmlSqlDatabaseWrapper>());
    if (!r || r->d()->type != Heap::QQmlSqlDatabaseWrapper::Database)
        V4THROW_REFERENCE("Not a SQLDatabase object");

    RETURN_RESULT(Encode(scope.engine->newString(*r->d()->version)));
}

static ReturnedValue qmlsqldatabase_rows_length(const FunctionObject *b, const Value *thisObject, const QV4::Value *, int)
{
    Scope scope(b);
    QV4::Scoped<QQmlSqlDatabaseWrapper> r(scope, thisObject->as<QQmlSqlDatabaseWrapper>());
    if (!r || r->d()->type != Heap::QQmlSqlDatabaseWrapper::Rows)
        V4THROW_REFERENCE("Not a SQLDatabase::Rows object");

    int s = r->d()->sqlQuery->size();
    if (s < 0) {
        // Inefficient
        if (r->d()->sqlQuery->last()) {
            s = r->d()->sqlQuery->at() + 1;
        } else {
            s = 0;
        }
    }
    RETURN_RESULT(Encode(s));
}

static ReturnedValue qmlsqldatabase_rows_forwardOnly(const FunctionObject *b, const Value *thisObject, const QV4::Value *, int)
{
    Scope scope(b);
    QV4::Scoped<QQmlSqlDatabaseWrapper> r(scope, thisObject->as<QQmlSqlDatabaseWrapper>());
    if (!r || r->d()->type != Heap::QQmlSqlDatabaseWrapper::Rows)
        V4THROW_REFERENCE("Not a SQLDatabase::Rows object");
    RETURN_RESULT(Encode(r->d()->sqlQuery->isForwardOnly()));
}

static ReturnedValue qmlsqldatabase_rows_setForwardOnly(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    Scope scope(b);
    QV4::Scoped<QQmlSqlDatabaseWrapper> r(scope, thisObject->as<QQmlSqlDatabaseWrapper>());
    if (!r || r->d()->type != Heap::QQmlSqlDatabaseWrapper::Rows)
        V4THROW_REFERENCE("Not a SQLDatabase::Rows object");
    if (argc < 1)
        RETURN_RESULT(scope.engine->throwTypeError());

    r->d()->sqlQuery->setForwardOnly(argv[0].toBoolean());
    RETURN_UNDEFINED();
}

QQmlSqlDatabaseData::~QQmlSqlDatabaseData()
{
}

static ReturnedValue qmlsqldatabase_rows_index(const QQmlSqlDatabaseWrapper *r, ExecutionEngine *v4, quint32 index, bool *hasProperty = nullptr)
{
    Scope scope(v4);

    if (r->d()->sqlQuery->at() == (int)index || r->d()->sqlQuery->seek(index)) {
        QSqlRecord record = r->d()->sqlQuery->record();
        // XXX optimize
        ScopedObject row(scope, v4->newObject());
        for (int ii = 0; ii < record.count(); ++ii) {
            QVariant v = record.value(ii);
            ScopedString s(scope, v4->newIdentifier(record.fieldName(ii)));
            ScopedValue val(scope, v.isNull() ? Encode::null() : v4->fromVariant(v));
            row->put(s.getPointer(), val);
        }
        if (hasProperty)
            *hasProperty = true;
        return row.asReturnedValue();
    } else {
        if (hasProperty)
            *hasProperty = false;
        return Encode::undefined();
    }
}

ReturnedValue QQmlSqlDatabaseWrapper::virtualGet(const Managed *m, PropertyKey id, const Value *receiver, bool *hasProperty)
{
    if (!id.isArrayIndex())
        return Object::virtualGet(m, id, receiver, hasProperty);

    uint index = id.asArrayIndex();
    Q_ASSERT(m->as<QQmlSqlDatabaseWrapper>());
    const QQmlSqlDatabaseWrapper *r = static_cast<const QQmlSqlDatabaseWrapper *>(m);
    if (!r || r->d()->type != Heap::QQmlSqlDatabaseWrapper::Rows)
        return Object::virtualGet(m, id, receiver, hasProperty);

    return qmlsqldatabase_rows_index(r, r->engine(), index, hasProperty);
}

static ReturnedValue qmlsqldatabase_rows_item(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    Scope scope(b);
    QV4::Scoped<QQmlSqlDatabaseWrapper> r(scope, thisObject->as<QQmlSqlDatabaseWrapper>());
    if (!r || r->d()->type != Heap::QQmlSqlDatabaseWrapper::Rows)
        V4THROW_REFERENCE("Not a SQLDatabase::Rows object");

    RETURN_RESULT(qmlsqldatabase_rows_index(r, scope.engine, argc ? argv[0].toUInt32() : 0));
}

static QVariant toSqlVariant(QV4::ExecutionEngine *engine, const QV4::ScopedValue &value)
{
    // toVariant() maps a null JS value to QVariant(VoidStar), but the SQL module
    // expects a null variant. (this is because of QTBUG-40880)
    if (value->isNull())
        return QVariant();
    return engine->toVariant(value, /*typehint*/-1);
}

static ReturnedValue qmlsqldatabase_executeSql(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    Scope scope(b);
    QV4::Scoped<QQmlSqlDatabaseWrapper> r(scope, thisObject->as<QQmlSqlDatabaseWrapper>());
    if (!r || r->d()->type != Heap::QQmlSqlDatabaseWrapper::Query)
        V4THROW_REFERENCE("Not a SQLDatabase::Query object");

    if (!r->d()->inTransaction)
        V4THROW_SQL(SQLEXCEPTION_DATABASE_ERR,QQmlEngine::tr("executeSql called outside transaction()"));

    QSqlDatabase db = *r->d()->database;

    QString sql = argc ? argv[0].toQString() : QString();

    if (r->d()->readonly && !sql.startsWith(QLatin1String("SELECT"),Qt::CaseInsensitive)) {
        V4THROW_SQL(SQLEXCEPTION_SYNTAX_ERR, QQmlEngine::tr("Read-only Transaction"));
    }

    QSqlQuery query(db);
    bool err = false;

    ScopedValue result(scope, Value::undefinedValue());

    if (query.prepare(sql)) {
        if (argc > 1) {
            ScopedValue values(scope, argv[1]);
            if (values->as<ArrayObject>()) {
                ScopedArrayObject array(scope, values);
                quint32 size = array->getLength();
                QV4::ScopedValue v(scope);
                for (quint32 ii = 0; ii < size; ++ii) {
                    query.bindValue(ii, toSqlVariant(scope.engine, (v = array->get(ii))));
                }
            } else if (values->as<Object>()) {
                ScopedObject object(scope, values);
                ObjectIterator it(scope, object, ObjectIterator::EnumerableOnly);
                ScopedValue key(scope);
                QV4::ScopedValue val(scope);
                while (1) {
                    key = it.nextPropertyName(val);
                    if (key->isNull())
                        break;
                    QVariant v = toSqlVariant(scope.engine, val);
                    if (key->isString()) {
                        query.bindValue(key->stringValue()->toQString(), v);
                    } else {
                        Q_ASSERT(key->isInteger());
                        query.bindValue(key->integerValue(), v);
                    }
                }
            } else {
                query.bindValue(0, toSqlVariant(scope.engine, values));
            }
        }
        if (query.exec()) {
            QV4::Scoped<QQmlSqlDatabaseWrapper> rows(scope, QQmlSqlDatabaseWrapper::create(scope.engine));
            QV4::ScopedObject p(scope, databaseData(scope.engine)->rowsProto.value());
            rows->setPrototypeUnchecked(p.getPointer());
            rows->d()->type = Heap::QQmlSqlDatabaseWrapper::Rows;
            *rows->d()->database = db;
            *rows->d()->sqlQuery = query;

            ScopedObject resultObject(scope, scope.engine->newObject());
            result = resultObject.asReturnedValue();
            // XXX optimize
            ScopedString s(scope);
            ScopedValue v(scope);
            resultObject->put((s = scope.engine->newIdentifier("rowsAffected")).getPointer(), (v = Value::fromInt32(query.numRowsAffected())));
            resultObject->put((s = scope.engine->newIdentifier("insertId")).getPointer(), (v = scope.engine->newString(query.lastInsertId().toString())));
            resultObject->put((s = scope.engine->newIdentifier("rows")).getPointer(), rows);
        } else {
            err = true;
        }
    } else {
        err = true;
    }
    if (err)
        V4THROW_SQL(SQLEXCEPTION_DATABASE_ERR,query.lastError().text());

    RETURN_RESULT(result->asReturnedValue());
}

struct TransactionRollback {
    QSqlDatabase *db;
    bool *inTransactionFlag;

    TransactionRollback(QSqlDatabase *database, bool *transactionFlag)
        : db(database)
        , inTransactionFlag(transactionFlag)
    {
        if (inTransactionFlag)
            *inTransactionFlag = true;
    }

    ~TransactionRollback()
    {
        if (inTransactionFlag)
            *inTransactionFlag = false;
        if (db)
            db->rollback();
    }

    void clear() {
        db = nullptr;
        if (inTransactionFlag)
            *inTransactionFlag = false;
        inTransactionFlag = nullptr;
    }
};


static ReturnedValue qmlsqldatabase_changeVersion(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc)
{
    Scope scope(b);
    if (argc < 2)
        RETURN_UNDEFINED();

    Scoped<QQmlSqlDatabaseWrapper> r(scope, *thisObject);
    if (!r || r->d()->type != Heap::QQmlSqlDatabaseWrapper::Database)
        V4THROW_REFERENCE("Not a SQLDatabase object");

    QSqlDatabase db = *r->d()->database;
    QString from_version = argv[0].toQString();
    QString to_version = argv[1].toQString();
    ScopedFunctionObject callback(scope, argc > 2 ? argv[2] : Value::undefinedValue());

    if (from_version != *r->d()->version)
        V4THROW_SQL(SQLEXCEPTION_VERSION_ERR, QQmlEngine::tr("Version mismatch: expected %1, found %2").arg(from_version).arg(*r->d()->version));

    Scoped<QQmlSqlDatabaseWrapper> w(scope, QQmlSqlDatabaseWrapper::create(scope.engine));
    ScopedObject p(scope, databaseData(scope.engine)->queryProto.value());
    w->setPrototypeUnchecked(p.getPointer());
    w->d()->type = Heap::QQmlSqlDatabaseWrapper::Query;
    *w->d()->database = db;
    *w->d()->version = *r->d()->version;

    bool ok = true;
    if (!!callback) {
        ok = false;
        db.transaction();

        JSCallData jsCall(scope, 1);
        *jsCall->thisObject = scope.engine->globalObject;
        jsCall->args[0] = w;

        TransactionRollback rollbackOnException(&db, &w->d()->inTransaction);
        callback->call(jsCall);
        rollbackOnException.clear();
        if (!db.commit()) {
            db.rollback();
            V4THROW_SQL(SQLEXCEPTION_UNKNOWN_ERR,QQmlEngine::tr("SQL transaction failed"));
        } else {
            ok = true;
        }
    }

    if (ok) {
        *w->d()->version = to_version;
#if QT_CONFIG(settings)
        const QQmlEnginePrivate *enginePrivate = QQmlEnginePrivate::get(scope.engine->qmlEngine());
        QSettings ini(enginePrivate->offlineStorageDatabaseDirectory() + db.connectionName() + QLatin1String(".ini"), QSettings::IniFormat);
        ini.setValue(QLatin1String("Version"), to_version);
#endif
    }

    RETURN_UNDEFINED();
}

static ReturnedValue qmlsqldatabase_transaction_shared(const FunctionObject *b, const Value *thisObject, const Value *argv, int argc, bool readOnly)
{
    Scope scope(b);
    QV4::Scoped<QQmlSqlDatabaseWrapper> r(scope, thisObject->as<QQmlSqlDatabaseWrapper>());
    if (!r || r->d()->type != Heap::QQmlSqlDatabaseWrapper::Database)
        V4THROW_REFERENCE("Not a SQLDatabase object");

    const FunctionObject *callback = argc ? argv[0].as<FunctionObject>() : nullptr;
    if (!callback)
        V4THROW_SQL(SQLEXCEPTION_UNKNOWN_ERR, QQmlEngine::tr("transaction: missing callback"));

    QSqlDatabase db = *r->d()->database;

    Scoped<QQmlSqlDatabaseWrapper> w(scope, QQmlSqlDatabaseWrapper::create(scope.engine));
    QV4::ScopedObject p(scope, databaseData(scope.engine)->queryProto.value());
    w->setPrototypeUnchecked(p.getPointer());
    w->d()->type = Heap::QQmlSqlDatabaseWrapper::Query;
    *w->d()->database = db;
    *w->d()->version = *r->d()->version;
    w->d()->readonly = readOnly;

    db.transaction();
    if (callback) {
        JSCallData jsCall(scope, 1);
        *jsCall->thisObject = scope.engine->globalObject;
        jsCall->args[0] = w;
        TransactionRollback rollbackOnException(&db, &w->d()->inTransaction);
        callback->call(jsCall);
        rollbackOnException.clear();

        if (!db.commit())
            db.rollback();
    }

    RETURN_UNDEFINED();
}

static ReturnedValue qmlsqldatabase_transaction(const FunctionObject *f, const Value *thisObject, const Value *argv, int argc)
{
    return qmlsqldatabase_transaction_shared(f, thisObject, argv, argc, false);
}

static ReturnedValue qmlsqldatabase_read_transaction(const FunctionObject *f, const Value *thisObject, const Value *argv, int argc)
{
    return qmlsqldatabase_transaction_shared(f, thisObject, argv, argc, true);
}

QQmlSqlDatabaseData::QQmlSqlDatabaseData(ExecutionEngine *v4)
{
    Scope scope(v4);
    {
        ScopedObject proto(scope, v4->newObject());
        proto->defineDefaultProperty(QStringLiteral("transaction"), qmlsqldatabase_transaction);
        proto->defineDefaultProperty(QStringLiteral("readTransaction"), qmlsqldatabase_read_transaction);
        proto->defineAccessorProperty(QStringLiteral("version"), qmlsqldatabase_version, nullptr);
        proto->defineDefaultProperty(QStringLiteral("changeVersion"), qmlsqldatabase_changeVersion);
        databaseProto = proto;
    }

    {
        ScopedObject proto(scope, v4->newObject());
        proto->defineDefaultProperty(QStringLiteral("executeSql"), qmlsqldatabase_executeSql);
        queryProto = proto;
    }
    {
        ScopedObject proto(scope, v4->newObject());
        proto->defineDefaultProperty(QStringLiteral("item"), qmlsqldatabase_rows_item);
        proto->defineAccessorProperty(QStringLiteral("length"), qmlsqldatabase_rows_length, nullptr);
        proto->defineAccessorProperty(QStringLiteral("forwardOnly"),
                                      qmlsqldatabase_rows_forwardOnly, qmlsqldatabase_rows_setForwardOnly);
        rowsProto = proto;
    }
}

/*
HTML5 "spec" says "rs.rows[n]", but WebKit only impelments "rs.rows.item(n)". We do both (and property iterator).
We add a "forwardOnly" property that stops Qt caching results (code promises to only go forward
through the data.
*/


/*!
    \qmlmodule QtQuick.LocalStorage 2.\QtMinorVersion
    \title Qt Quick Local Storage QML Types
    \ingroup qmlmodules
    \brief Provides a JavaScript object singleton type for accessing a local
    SQLite database

    This is a singleton type for reading and writing to SQLite databases.


    \section1 Methods

    \list
    \li object \b{\l{#openDatabaseSync}{openDatabaseSync}}(string name, string version, string description, int estimated_size, jsobject callback(db))
    \endlist


    \section1 Detailed Description

    To use the types in this module, import the module and call the
    relevant functions using the \c LocalStorage type:

    \qml \QtMinorVersion
    import QtQuick 2.\1
    import QtQuick.LocalStorage 2.\1

    Item {
        Component.onCompleted: {
            var db = LocalStorage.openDatabaseSync(...)
        }
    }
    \endqml


These databases are user-specific and QML-specific, but accessible to all QML applications.
They are stored in the \c Databases subdirectory
of QQmlEngine::offlineStoragePath(), currently as SQLite databases.

Database connections are automatically closed during Javascript garbage collection.

The API can be used from JavaScript functions in your QML:

\snippet qml/localstorage/hello.qml 0

The API conforms to the Synchronous API of the HTML5 Web Database API,
\link http://www.w3.org/TR/2009/WD-webdatabase-20091029/ W3C Working Draft 29 October 2009\endlink.

The \l{Qt Quick Examples - Local Storage}{SQL Local Storage example} demonstrates the basics of
using the Offline Storage API.

\section3 Open or Create a Database

\qml \QtMinorVersion
import QtQuick.LocalStorage 2.\1 as Sql

db = Sql.openDatabaseSync(identifier, version, description, estimated_size, callback(db))
\endqml

The above code returns the database identified by \e identifier. If the database does not already exist, it
is created, and the function \e callback is called with the database as a parameter. \e identifier is the
name of the physical file (with or without full path) containing the database.  \e description and
\e estimated_size are written to the INI file (described below), but are currently unused.

May throw exception with code property SQLException.DATABASE_ERR, or SQLException.VERSION_ERR.

When a database is first created, an INI file is also created specifying its characteristics:

\table
\header \li \b {Key} \li \b {Value}
\row \li Identifier \li The name of the database passed to \c openDatabase()
\row \li Version \li The version of the database passed to \c openDatabase()
\row \li Description \li The description of the database passed to \c openDatabase()
\row \li EstimatedSize \li The estimated size (in bytes) of the database passed to \c openDatabase()
\row \li Driver \li Currently "QSQLITE"
\endtable

This data can be used by application tools.

\section3 db.changeVersion(from, to, callback(tx))

This method allows you to perform a \e{Scheme Upgrade}.

If the current version of \e db is not \e from, then an exception is thrown.

Otherwise, a database transaction is created and passed to \e callback. In this function,
you can call \e executeSql on \e tx to upgrade the database.

May throw exception with code property SQLException.DATABASE_ERR or SQLException.UNKNOWN_ERR.

See example below.

\badcode
    var db = LocalStorage.openDatabaseSync("ActivityTrackDB", "", "Database tracking sports activities", 1000000);
    if (db.version == "0.1") {
        db.changeVersion("0.1", "0.2", function(tx) {
            tx.executeSql("INSERT INTO trip_log VALUES(?, ?, ?)",
                        [ "01/10/2016","Sylling - Vikersund", "53" ]);
        }
    });
\endcode

\section3 db.transaction(callback(tx))

This method creates a read/write transaction and passed to \e callback. In this function,
you can call \e executeSql on \e tx to read and modify the database.

If the callback throws exceptions, the transaction is rolled back.
Below you will find an example of a database transaction which catches exceptions.


\quotefromfile localstorage/localstorage/Database.js
\skipuntil dbInit()
\printto dbGetHandle

In the example you can see an \c insert statement where values are assigned to the fields,
and the record is written into the table. That is an \c insert statement with a syntax that is usual
for a relational database. It is however also possible to work with JSON objects and
store them in a table.

Let's suppose a simple example where we store trips in JSON format using \c date as the unique key.
An example of a table that could be used for that purpose:

\badcode
    create table trip_log(date text, data text)
\endcode

The assignment of values to a JSON object:

\badcode
    var obj = {description = "Vikersund - Noresund", distance = "60"}
\endcode

In that case, the data could be saved in the following way:

\badcode
    db.transaction(function(tx) {
        result = tx.executeSQL("insert into trip_log values (?,?)",
                               ["01/11/2016", JSON.stringify(obj)])

\endcode

\section3 db.readTransaction(callback(tx))

This method creates a read-only transaction and passed to \e callback. In this function,
you can call \e executeSql on \e tx to read the database (with \c select statements).

\section3 results = tx.executeSql(statement, values)

This method executes an SQL \e statement, binding the list of \e values to SQL positional parameters ("?").

It returns a results object, with the following properties:

\table
\header \li \b {Type} \li \b {Property} \li \b {Value} \li \b {Applicability}
\row \li int \li rows.length \li The number of rows in the result \li SELECT
\row \li var \li rows.item(i) \li Function that returns row \e i of the result \li SELECT
\row \li int \li rowsAffected \li The number of rows affected by a modification \li UPDATE, DELETE
\row \li string \li insertId \li The id of the row inserted \li INSERT
\endtable

May throw exception with code property SQLException.DATABASE_ERR, SQLException.SYNTAX_ERR, or SQLException.UNKNOWN_ERR.

See below for an example:

\quotefromfile localstorage/localstorage/Database.js
\skipto dbReadAll()
\printto dbUpdate(Pdate

\section1 Method Documentation

\target openDatabaseSync
\code
object openDatabaseSync(string name, string version, string description, int estimated_size, jsobject callback(db))
\endcode

Opens or creates a local storage sql database by the given parameters.

\list
\li \c name is the database name
\li \c version is the database version
\li \c description is the database display name
\li \c estimated_size is the database's estimated size, in bytes
\li \c callback is an optional parameter, which is invoked if the database has not yet been created.
\endlist

Returns the created database object.

*/
class QQuickLocalStorage : public QObject
{
    Q_OBJECT
public:
    QQuickLocalStorage(QObject *parent=nullptr) : QObject(parent)
    {
    }
    ~QQuickLocalStorage() override {
    }

   Q_INVOKABLE void openDatabaseSync(QQmlV4Function* args);
};

void QQuickLocalStorage::openDatabaseSync(QQmlV4Function *args)
{
#if QT_CONFIG(settings)
    QV4::Scope scope(args->v4engine());
    if (scope.engine->qmlEngine()->offlineStoragePath().isEmpty())
        V4THROW_SQL2(SQLEXCEPTION_DATABASE_ERR, QQmlEngine::tr("SQL: can't create database, offline storage is disabled."));

    QV4::ScopedValue v(scope);
    QString dbname = (v = (*args)[0])->toQStringNoThrow();
    QString dbversion = (v = (*args)[1])->toQStringNoThrow();
    QString dbdescription = (v = (*args)[2])->toQStringNoThrow();
    int dbestimatedsize = (v = (*args)[3])->toInt32();
    FunctionObject *dbcreationCallback = (v = (*args)[4])->as<FunctionObject>();
    QString basename = args->v4engine()->qmlEngine()->offlineStorageDatabaseFilePath(dbname);
    QFileInfo dbFile(basename);
    if (!QDir().mkpath(dbFile.dir().absolutePath())) {
        const QString message = QQmlEngine::tr("LocalStorage: can't create path %1").
                                arg(QDir::toNativeSeparators(dbFile.dir().absolutePath()));
        V4THROW_SQL2(SQLEXCEPTION_DATABASE_ERR, message);
    }
    QString dbid = dbFile.fileName();
    bool created = false;
    QString version = dbversion;
    QSqlDatabase database;

    {
        QSettings ini(basename+QLatin1String(".ini"),QSettings::IniFormat);

        if (QSqlDatabase::connectionNames().contains(dbid)) {
            database = QSqlDatabase::database(dbid);
            version = ini.value(QLatin1String("Version")).toString();
            if (version != dbversion && !dbversion.isEmpty() && !version.isEmpty())
                V4THROW_SQL2(SQLEXCEPTION_VERSION_ERR, QQmlEngine::tr("SQL: database version mismatch"));
        } else {
            created = !QFile::exists(basename+QLatin1String(".sqlite"));
            if (created) {
                ini.setValue(QLatin1String("Name"), dbname);
                if (dbcreationCallback)
                    version = QString();
                ini.setValue(QLatin1String("Version"), version);
                ini.setValue(QLatin1String("Description"), dbdescription);
                ini.setValue(QLatin1String("EstimatedSize"), dbestimatedsize);
                ini.setValue(QLatin1String("Driver"), QLatin1String("QSQLITE"));
            } else {
                if (!dbversion.isEmpty() && ini.value(QLatin1String("Version")) != dbversion) {
                    // Incompatible
                    V4THROW_SQL2(SQLEXCEPTION_VERSION_ERR,QQmlEngine::tr("SQL: database version mismatch"));
                }
                version = ini.value(QLatin1String("Version")).toString();
            }
            database = QSqlDatabase::addDatabase(QLatin1String("QSQLITE"), dbid);
            database.setDatabaseName(basename+QLatin1String(".sqlite"));
        }
        if (!database.isOpen())
            database.open();
    }

    QV4::Scoped<QQmlSqlDatabaseWrapper> db(scope, QQmlSqlDatabaseWrapper::create(scope.engine));
    QV4::ScopedObject p(scope, databaseData(scope.engine)->databaseProto.value());
    db->setPrototypeUnchecked(p.getPointer());
    *db->d()->database = database;
    *db->d()->version = version;

    if (created && dbcreationCallback) {
        JSCallData jsCall(scope, 1);
        *jsCall->thisObject = scope.engine->globalObject;
        jsCall->args[0] = db;
        dbcreationCallback->call(jsCall);
    }

    args->setReturnValue(db.asReturnedValue());
#else
    Q_UNUSED(args)
#endif // settings
}

static QObject *module_api_factory(QQmlEngine *engine, QJSEngine *scriptEngine)
{
   Q_UNUSED(engine)
   Q_UNUSED(scriptEngine)
   QQuickLocalStorage *api = new QQuickLocalStorage();

   return api;
}

class QQmlLocalStoragePlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)

public:
    QQmlLocalStoragePlugin(QObject *parent = nullptr) : QQmlExtensionPlugin(parent)
    {
    }
    void registerTypes(const char *uri) override
    {
        Q_ASSERT(QLatin1String(uri) == QLatin1String("QtQuick.LocalStorage"));
        qmlRegisterSingletonType<QQuickLocalStorage>(uri, 2, 0, "LocalStorage", module_api_factory);

        // Auto-increment the import to stay in sync with ALL future QtQuick minor versions from 5.11 onward
        qmlRegisterModule(uri, 2, QT_VERSION_MINOR);
    }
};

QT_END_NAMESPACE

#include "plugin.moc"
