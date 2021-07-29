/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Designer of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "formbuilderextra_p.h"
#include "abstractformbuilder.h"
#include "resourcebuilder_p.h"
#include "textbuilder_p.h"
#include "ui4_p.h"

#include <QtWidgets/qlabel.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qgridlayout.h>

#include <QtCore/qvariant.h>
#include <QtCore/qdebug.h>
#include <QtCore/qtextstream.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qcoreapplication.h>
#include <QtCore/qversionnumber.h>

#include <limits.h>

QT_BEGIN_NAMESPACE

#ifdef QFORMINTERNAL_NAMESPACE
namespace QFormInternal {
#endif

void uiLibWarning(const QString &message) {
    qWarning("Designer: %s", qPrintable(message));
}


QFormBuilderExtra::CustomWidgetData::CustomWidgetData() :
    isContainer(false)
{
}

QFormBuilderExtra::CustomWidgetData::CustomWidgetData(const DomCustomWidget *dcw) :
    addPageMethod(dcw->elementAddPageMethod()),
    baseClass(dcw->elementExtends()),
    isContainer(dcw->hasElementContainer() && dcw->elementContainer() != 0)
{
}

QFormBuilderExtra::QFormBuilderExtra() :
    m_defaultMargin(INT_MIN),
    m_defaultSpacing(INT_MIN),
    m_language(QStringLiteral("c++")),
    m_layoutWidget(false),
    m_resourceBuilder(0),
    m_textBuilder(0)
{
}

QFormBuilderExtra::~QFormBuilderExtra()
{
    clearResourceBuilder();
    clearTextBuilder();
}

void QFormBuilderExtra::clear()
{
    m_buddies.clear();
    m_parentWidget = 0;
    m_parentWidgetIsSet = false;
    m_customWidgetDataHash.clear();
    m_buttonGroups.clear();
}

static inline QString msgXmlError(const QXmlStreamReader &reader)
{
    return QCoreApplication::translate("QAbstractFormBuilder",
                                       "An error has occurred while reading the UI file at line %1, column %2: %3")
                                       .arg(reader.lineNumber()).arg(reader.columnNumber())
                                       .arg(reader.errorString());
}

// Read and check the  version and the (optional) language attribute
// of an <ui> element and leave reader positioned at <ui>.
static bool inline readUiAttributes(QXmlStreamReader &reader, const QString &language,
                                    QString *errorMessage)
{
    const QString uiElement = QStringLiteral("ui");
    // Read up to first element
    while (!reader.atEnd()) {
        switch (reader.readNext()) {
        case QXmlStreamReader::Invalid:
            *errorMessage = msgXmlError(reader);
            return false;
        case QXmlStreamReader::StartElement:
            if (reader.name().compare(uiElement, Qt::CaseInsensitive) == 0) {
                const QString versionAttribute = QStringLiteral("version");
                const QString languageAttribute = QStringLiteral("language");
                const QXmlStreamAttributes attributes = reader.attributes();
                if (attributes.hasAttribute(versionAttribute)) {
                    const QVersionNumber version =
                        QVersionNumber::fromString(attributes.value(versionAttribute));
                    if (version < QVersionNumber(4)) {
                        *errorMessage =
                            QCoreApplication::translate("QAbstractFormBuilder",
                                                        "This file was created using Designer from Qt-%1 and cannot be read.")
                                                        .arg(attributes.value(versionAttribute));
                        return false;
                    } // version error
                }     // has version
                if (attributes.hasAttribute(languageAttribute)) {
                    // Check on optional language (Jambi)
                    const QString formLanguage = attributes.value(languageAttribute).toString();
                    if (!formLanguage.isEmpty() && formLanguage.compare(language, Qt::CaseInsensitive)) {
                        *errorMessage =
                            QCoreApplication::translate("QAbstractFormBuilder",
                                                        "This file cannot be read because it was created using %1.")
                                                        .arg(formLanguage);
                        return false;
                    } // language error
                }    // has language
                return true;
            }  // <ui> matched
            break;
        default:
            break;
        }
    }
    // No <ui> found.
    *errorMessage = QCoreApplication::translate("QAbstractFormBuilder",
                                                "Invalid UI file: The root element <ui> is missing.");
    return false;
}

DomUI *QFormBuilderExtra::readUi(QIODevice *dev)
{
    QXmlStreamReader reader(dev);
    m_errorString.clear();
    if (!readUiAttributes(reader, m_language, &m_errorString)) {
        uiLibWarning(m_errorString);
        return nullptr;
    }
    DomUI *ui = new DomUI;
    ui->read(reader);
    if (reader.hasError()) {
        m_errorString = msgXmlError(reader);
        uiLibWarning(m_errorString);
        delete ui;
        return nullptr;
    }
    return ui;
}

QString QFormBuilderExtra::msgInvalidUiFile()
{
    return QCoreApplication::translate("QAbstractFormBuilder", "Invalid UI file");
}

bool QFormBuilderExtra::applyPropertyInternally(QObject *o, const QString &propertyName, const QVariant &value)
{
    // Store buddies and apply them later on as the widgets might not exist yet.
    QLabel *label = qobject_cast<QLabel*>(o);
    if (!label || propertyName != QFormBuilderStrings::instance().buddyProperty)
        return false;

    m_buddies.insert(label, value.toString());
    return true;
}

void QFormBuilderExtra::applyInternalProperties() const
{
    if (m_buddies.empty())
        return;

    const BuddyHash::const_iterator cend = m_buddies.constEnd();
    for (BuddyHash::const_iterator it = m_buddies.constBegin(); it != cend; ++it )
        applyBuddy(it.value(), BuddyApplyAll, it.key());
}

bool QFormBuilderExtra::applyBuddy(const QString &buddyName, BuddyMode applyMode, QLabel *label)
{
    if (buddyName.isEmpty()) {
        label->setBuddy(0);
        return false;
    }

    const QWidgetList widgets = label->topLevelWidget()->findChildren<QWidget*>(buddyName);
    if (widgets.empty()) {
        label->setBuddy(0);
        return false;
    }

    const QWidgetList::const_iterator cend = widgets.constEnd();
    for ( QWidgetList::const_iterator it =  widgets.constBegin(); it !=  cend; ++it) {
        if (applyMode == BuddyApplyAll || !(*it)->isHidden()) {
            label->setBuddy(*it);
            return true;
        }
    }

    label->setBuddy(0);
    return false;
}

const QPointer<QWidget> &QFormBuilderExtra::parentWidget() const
{
    return m_parentWidget;
}

bool QFormBuilderExtra::parentWidgetIsSet() const
{
    return m_parentWidgetIsSet;
}

void QFormBuilderExtra::setParentWidget(const QPointer<QWidget> &w)
{
    // Parent widget requires special handling of the geometry property.
    m_parentWidget = w;
    m_parentWidgetIsSet = true;
}

void QFormBuilderExtra::storeCustomWidgetData(const QString &className, const DomCustomWidget *d)
{
    if (d)
        m_customWidgetDataHash.insert(className, CustomWidgetData(d));
}

QString QFormBuilderExtra::customWidgetBaseClass(const QString &className) const
{
    const QHash<QString, CustomWidgetData>::const_iterator it = m_customWidgetDataHash.constFind(className);
    if (it != m_customWidgetDataHash.constEnd())
            return it.value().baseClass;
    return QString();
}

QString QFormBuilderExtra::customWidgetAddPageMethod(const QString &className) const
{
    const QHash<QString, CustomWidgetData>::const_iterator it = m_customWidgetDataHash.constFind(className);
    if (it != m_customWidgetDataHash.constEnd())
        return it.value().addPageMethod;
    return QString();
}

bool QFormBuilderExtra::isCustomWidgetContainer(const QString &className) const
{
    const QHash<QString, CustomWidgetData>::const_iterator it = m_customWidgetDataHash.constFind(className);
    if (it != m_customWidgetDataHash.constEnd())
        return it.value().isContainer;
    return false;
}

void QFormBuilderExtra::setProcessingLayoutWidget(bool processing)
{
    m_layoutWidget = processing;
}

 bool QFormBuilderExtra::processingLayoutWidget() const
{
    return m_layoutWidget;
}
void QFormBuilderExtra::setResourceBuilder(QResourceBuilder *builder)
{
    if (m_resourceBuilder == builder)
        return;
    clearResourceBuilder();
    m_resourceBuilder = builder;
}

QResourceBuilder *QFormBuilderExtra::resourceBuilder() const
{
    return m_resourceBuilder;
}

void QFormBuilderExtra::clearResourceBuilder()
{
    if (m_resourceBuilder) {
        delete m_resourceBuilder;
        m_resourceBuilder = 0;
    }
}

void QFormBuilderExtra::setTextBuilder(QTextBuilder *builder)
{
    if (m_textBuilder == builder)
        return;
    clearTextBuilder();
    m_textBuilder = builder;
}

QTextBuilder *QFormBuilderExtra::textBuilder() const
{
    return m_textBuilder;
}

void QFormBuilderExtra::clearTextBuilder()
{
    if (m_textBuilder) {
        delete m_textBuilder;
        m_textBuilder = 0;
    }
}

void QFormBuilderExtra::registerButtonGroups(const DomButtonGroups *domGroups)
{
    const auto &domGroupList = domGroups->elementButtonGroup();
    for (DomButtonGroup *domGroup : domGroupList)
        m_buttonGroups.insert(domGroup->attributeName(), ButtonGroupEntry(domGroup, nullptr));
}

// Utilities for parsing per-cell integer properties that have setters and
//  getters of the form 'setX(int idx, int value)' and 'x(int index)'
// (converting them to comma-separated string lists and back).
// Used for layout stretch and grid per-row/column properties.

// Format a list of cell-properties of one dimension as a ','-separated list
template <class Layout>
inline QString perCellPropertyToString(const Layout *l, int count, int (Layout::*getter)(int) const)
{
    if (count == 0)
        return QString();
    QString rc;
    {
        QTextStream str(&rc);
        for (int i = 0; i < count; i++) {
            if (i)
                str << QLatin1Char(',');
            str << (l->*getter)(i);
        }
    }
    return rc;
}

// Clear the property, set all cells to 0

template <class Layout>
inline void clearPerCellValue(Layout *l, int count, void (Layout::*setter)(int,int), int value = 0)
{
    for (int i = 0; i < count; i++)
        (l->*setter)(i, value);
}

// Parse and set the property from a comma-separated list

template <class Layout>
inline bool parsePerCellProperty(Layout *l, int count, void (Layout::*setter)(int,int), const QString &s, int defaultValue = 0)
{
    if (s.isEmpty()) {
        clearPerCellValue(l, count, setter, defaultValue);
        return true;
    }
    const QVector<QStringRef> list = s.splitRef(QLatin1Char(','));
    if (list.empty()) {
        clearPerCellValue(l, count, setter, defaultValue);
        return true;
    }
    // Apply all values contained in list
    const int ac = qMin(count, list.size());
    bool ok;
    int i = 0;
    for ( ; i < ac; i++) {
        const int value = list.at(i).toInt(&ok);
        if (!ok || value < 0)
            return false;
        (l->*setter)(i, value);
    }
    // Clear rest
    for ( ; i < count; i++)
        (l->*setter)(i, defaultValue);
    return true;
}

// Read and write stretch
static QString msgInvalidStretch(const QString &objectName, const QString &stretch)
{
    //: Parsing layout stretch values
    return QCoreApplication::translate("FormBuilder", "Invalid stretch value for '%1': '%2'").arg(objectName, stretch);
}

QString QFormBuilderExtra::boxLayoutStretch(const QBoxLayout *box)
{
     return perCellPropertyToString(box, box->count(), &QBoxLayout::stretch);
}

bool QFormBuilderExtra::setBoxLayoutStretch(const QString &s, QBoxLayout *box)
{
    const bool rc = parsePerCellProperty(box, box->count(), &QBoxLayout::setStretch, s);
    if (!rc)
        uiLibWarning(msgInvalidStretch(box->objectName(), s));
    return rc;
}

void QFormBuilderExtra::clearBoxLayoutStretch(QBoxLayout *box)
{
    clearPerCellValue(box, box->count(), &QBoxLayout::setStretch);
}

QString QFormBuilderExtra::gridLayoutRowStretch(const QGridLayout *grid)
{
    return perCellPropertyToString(grid, grid->rowCount(), &QGridLayout::rowStretch);
}

bool QFormBuilderExtra::setGridLayoutRowStretch(const QString &s, QGridLayout *grid)
{
    const bool rc = parsePerCellProperty(grid, grid->rowCount(), &QGridLayout::setRowStretch, s);
    if (!rc)
        uiLibWarning(msgInvalidStretch(grid->objectName(), s));
    return rc;
}

void QFormBuilderExtra::clearGridLayoutRowStretch(QGridLayout *grid)
{
    clearPerCellValue(grid, grid->rowCount(), &QGridLayout::setRowStretch);
}

QString QFormBuilderExtra::gridLayoutColumnStretch(const QGridLayout *grid)
{
    return perCellPropertyToString(grid, grid->columnCount(), &QGridLayout::columnStretch);
}

bool QFormBuilderExtra::setGridLayoutColumnStretch(const QString &s, QGridLayout *grid)
{
    const bool rc = parsePerCellProperty(grid, grid->columnCount(), &QGridLayout::setColumnStretch, s);
    if (!rc)
        uiLibWarning(msgInvalidStretch(grid->objectName(), s));
    return rc;
}

void QFormBuilderExtra::clearGridLayoutColumnStretch(QGridLayout *grid)
{
    clearPerCellValue(grid, grid->columnCount(), &QGridLayout::setColumnStretch);
}

// Read and write grid layout row/column size limits

static QString msgInvalidMinimumSize(const QString &objectName, const QString &ms)
{
    //: Parsing grid layout minimum size values
    return QCoreApplication::translate("FormBuilder", "Invalid minimum size for '%1': '%2'").arg(objectName, ms);
}

QString QFormBuilderExtra::gridLayoutRowMinimumHeight(const QGridLayout *grid)
{
    return perCellPropertyToString(grid, grid->rowCount(), &QGridLayout::rowMinimumHeight);
}

bool QFormBuilderExtra::setGridLayoutRowMinimumHeight(const QString &s, QGridLayout *grid)
{
    const bool rc = parsePerCellProperty(grid, grid->rowCount(), &QGridLayout::setRowMinimumHeight, s);
    if (!rc)
        uiLibWarning(msgInvalidMinimumSize(grid->objectName(), s));
    return rc;
}

void QFormBuilderExtra::clearGridLayoutRowMinimumHeight(QGridLayout *grid)
{
     clearPerCellValue(grid, grid->rowCount(), &QGridLayout::setRowMinimumHeight);
}

QString QFormBuilderExtra::gridLayoutColumnMinimumWidth(const QGridLayout *grid)
{
    return perCellPropertyToString(grid, grid->columnCount(), &QGridLayout::columnMinimumWidth);
}

bool QFormBuilderExtra::setGridLayoutColumnMinimumWidth(const QString &s, QGridLayout *grid)
{
    const bool rc = parsePerCellProperty(grid, grid->columnCount(), &QGridLayout::setColumnMinimumWidth, s);
    if (!rc)
        uiLibWarning(msgInvalidMinimumSize(grid->objectName(), s));
    return rc;
}

void QFormBuilderExtra::clearGridLayoutColumnMinimumWidth(QGridLayout *grid)
{
    clearPerCellValue(grid, grid->columnCount(), &QGridLayout::setColumnMinimumWidth);
}

// ------------ QFormBuilderStrings

QFormBuilderStrings::QFormBuilderStrings() :
    buddyProperty(QStringLiteral("buddy")),
    cursorProperty(QStringLiteral("cursor")),
    objectNameProperty(QStringLiteral("objectName")),
    trueValue(QStringLiteral("true")),
    falseValue(QStringLiteral("false")),
    horizontalPostFix(QStringLiteral("Horizontal")),
    separator(QStringLiteral("separator")),
    defaultTitle(QStringLiteral("Page")),
    titleAttribute(QStringLiteral("title")),
    labelAttribute(QStringLiteral("label")),
    toolTipAttribute(QStringLiteral("toolTip")),
    whatsThisAttribute(QStringLiteral("whatsThis")),
    flagsAttribute(QStringLiteral("flags")),
    iconAttribute(QStringLiteral("icon")),
    pixmapAttribute(QStringLiteral("pixmap")),
    textAttribute(QStringLiteral("text")),
    currentIndexProperty(QStringLiteral("currentIndex")),
    toolBarAreaAttribute(QStringLiteral("toolBarArea")),
    toolBarBreakAttribute(QStringLiteral("toolBarBreak")),
    dockWidgetAreaAttribute(QStringLiteral("dockWidgetArea")),
    marginProperty(QStringLiteral("margin")),
    spacingProperty(QStringLiteral("spacing")),
    leftMarginProperty(QStringLiteral("leftMargin")),
    topMarginProperty(QStringLiteral("topMargin")),
    rightMarginProperty(QStringLiteral("rightMargin")),
    bottomMarginProperty(QStringLiteral("bottomMargin")),
    horizontalSpacingProperty(QStringLiteral("horizontalSpacing")),
    verticalSpacingProperty(QStringLiteral("verticalSpacing")),
    sizeHintProperty(QStringLiteral("sizeHint")),
    sizeTypeProperty(QStringLiteral("sizeType")),
    orientationProperty(QStringLiteral("orientation")),
    styleSheetProperty(QStringLiteral("styleSheet")),
    qtHorizontal(QStringLiteral("Qt::Horizontal")),
    qtVertical(QStringLiteral("Qt::Vertical")),
    currentRowProperty(QStringLiteral("currentRow")),
    tabSpacingProperty(QStringLiteral("tabSpacing")),
    qWidgetClass(QStringLiteral("QWidget")),
    lineClass(QStringLiteral("Line")),
    geometryProperty(QStringLiteral("geometry")),
    scriptWidgetVariable(QStringLiteral("widget")),
    scriptChildWidgetsVariable(QStringLiteral("childWidgets"))
{
    itemRoles.append(qMakePair(Qt::FontRole, QString::fromLatin1("font")));
    itemRoles.append(qMakePair(Qt::TextAlignmentRole, QString::fromLatin1("textAlignment")));
    itemRoles.append(qMakePair(Qt::BackgroundRole, QString::fromLatin1("background")));
    itemRoles.append(qMakePair(Qt::ForegroundRole, QString::fromLatin1("foreground")));
    itemRoles.append(qMakePair(Qt::CheckStateRole, QString::fromLatin1("checkState")));

    for (const RoleNName &it : qAsConst(itemRoles))
        treeItemRoleHash.insert(it.second, it.first);

    itemTextRoles.append(qMakePair(qMakePair(Qt::EditRole, Qt::DisplayPropertyRole),
                                   textAttribute)); // This must be first for the loop below
    itemTextRoles.append(qMakePair(qMakePair(Qt::ToolTipRole, Qt::ToolTipPropertyRole),
                                   toolTipAttribute));
    itemTextRoles.append(qMakePair(qMakePair(Qt::StatusTipRole, Qt::StatusTipPropertyRole),
                                   QString::fromLatin1("statusTip")));
    itemTextRoles.append(qMakePair(qMakePair(Qt::WhatsThisRole, Qt::WhatsThisPropertyRole),
                                   whatsThisAttribute));

    // Note: this skips the first item!
    QList<TextRoleNName>::const_iterator it = itemTextRoles.constBegin(), end = itemTextRoles.constEnd();
    while (++it != end)
        treeItemTextRoleHash.insert(it->second, it->first);
}

const QFormBuilderStrings &QFormBuilderStrings::instance()
{
    static const QFormBuilderStrings rc;
    return rc;
}

#ifdef QFORMINTERNAL_NAMESPACE
} // namespace QFormInternal
#endif

QT_END_NAMESPACE
