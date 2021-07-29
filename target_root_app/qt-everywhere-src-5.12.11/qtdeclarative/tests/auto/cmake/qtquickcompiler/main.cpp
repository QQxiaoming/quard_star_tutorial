
#include <QtCore>
#include <QtQml>
#include <QtTest>

class tst_QQC : public QObject
{
    Q_OBJECT
private slots:
    void packaging();
};

void tst_QQC::packaging()
{
    QVERIFY(QFile::exists(":/main.qml"));
    QCOMPARE(QFileInfo(":/main.qml").size(), 0);
    QVERIFY(QFile::exists(":/main.cpp"));
    QVERIFY(QFileInfo(":/main.cpp").size() > 0);


    QQmlEngine engine;
    QQmlComponent component(&engine, QUrl("qrc:/main.qml"));
    QScopedPointer<QObject> obj(component.create());
    QVERIFY(!obj.isNull());
    QCOMPARE(obj->property("success").toInt(), 42);
}

QTEST_MAIN(tst_QQC)

#include "main.moc"
