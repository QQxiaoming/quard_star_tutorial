/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the tools applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "distancefieldmodel.h"

#include <QtCore/qdir.h>
#include <QtCore/qdatastream.h>
#include <QtCore/qmath.h>
#include <QtCore/qendian.h>
#include <QtCore/qbuffer.h>
#include <QtGui/qdesktopservices.h>
#include <QtGui/qrawfont.h>
#include <QtWidgets/qmessagebox.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qprogressbar.h>
#include <QtWidgets/qfiledialog.h>
#include <QtWidgets/qinputdialog.h>

#include <QtCore/private/qunicodetables_p.h>
#include <QtGui/private/qdistancefield_p.h>
#include <QtQuick/private/qsgareaallocator_p.h>
#include <QtQuick/private/qsgadaptationlayer_p.h>

QT_BEGIN_NAMESPACE

static void openHelp()
{
    QDesktopServices::openUrl(QUrl(QLatin1String("http://doc.qt.io/qt-5/qtdistancefieldgenerator-index.html")));
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_settings(qApp->organizationName(), qApp->applicationName())
    , m_model(new DistanceFieldModel(this))
    , m_statusBarLabel(nullptr)
    , m_statusBarProgressBar(nullptr)
{
    ui->setupUi(this);
    ui->lvGlyphs->setModel(m_model);

    ui->actionHelp->setShortcut(QKeySequence::HelpContents);

    m_statusBarLabel = new QLabel(this);
    m_statusBarLabel->setText(tr("Ready"));
    ui->statusbar->addPermanentWidget(m_statusBarLabel);

    m_statusBarProgressBar = new QProgressBar(this);
    ui->statusbar->addPermanentWidget(m_statusBarProgressBar);
    m_statusBarProgressBar->setVisible(false);

    if (m_settings.contains(QStringLiteral("fontDirectory")))
        m_fontDir = m_settings.value(QStringLiteral("fontDirectory")).toString();
    else
        m_fontDir = QDir::currentPath();

    qRegisterMetaType<glyph_t>("glyph_t");
    qRegisterMetaType<QPainterPath>("QPainterPath");

    restoreGeometry(m_settings.value(QStringLiteral("geometry")).toByteArray());

    setupConnections();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::open(const QString &path)
{
    m_fileName.clear();
    m_fontFile = path;
    m_fontDir = QFileInfo(path).absolutePath();
    m_settings.setValue(QStringLiteral("fontDirectory"), m_fontDir);

    ui->lwUnicodeRanges->clear();
    ui->lwUnicodeRanges->setDisabled(true);
    ui->action_Save->setDisabled(true);
    ui->action_Save_as->setDisabled(true);
    ui->tbSave->setDisabled(true);
    ui->action_Open->setDisabled(true);
    m_model->setFont(path);
}

void MainWindow::closeEvent(QCloseEvent * /*event*/)
{
    m_settings.setValue(QStringLiteral("geometry"), saveGeometry());
}

void MainWindow::setupConnections()
{
    connect(ui->action_Open, &QAction::triggered, this, &MainWindow::openFont);
    connect(ui->actionE_xit, &QAction::triggered, qApp, &QApplication::quit);
    connect(ui->action_Save, &QAction::triggered, this, &MainWindow::save);
    connect(ui->action_Save_as, &QAction::triggered, this, &MainWindow::saveAs);
    connect(ui->tbSave, &QToolButton::clicked, this, &MainWindow::save);
    connect(ui->tbSelectAll, &QToolButton::clicked, this, &MainWindow::selectAll);
    connect(ui->actionSelect_all, &QAction::triggered, this, &MainWindow::selectAll);
    connect(ui->actionSelect_string, &QAction::triggered, this, &MainWindow::selectString);
    connect(ui->actionHelp, &QAction::triggered, this, openHelp);
    connect(ui->actionAbout_App, &QAction::triggered, this, &MainWindow::about);
    connect(ui->actionAbout_Qt, &QAction::triggered, this, [this]() {
        QMessageBox::aboutQt(this);
    });
    connect(ui->lwUnicodeRanges, &QListWidget::itemSelectionChanged, this, &MainWindow::updateUnicodeRanges);

    connect(ui->lvGlyphs->selectionModel(),
            &QItemSelectionModel::selectionChanged,
            this,
            &MainWindow::updateSelection);
    connect(m_model, &DistanceFieldModel::startGeneration, this, &MainWindow::startProgressBar);
    connect(m_model, &DistanceFieldModel::stopGeneration, this, &MainWindow::stopProgressBar);
    connect(m_model, &DistanceFieldModel::distanceFieldGenerated, this, &MainWindow::updateProgressBar);
    connect(m_model, &DistanceFieldModel::stopGeneration, this, &MainWindow::populateUnicodeRanges);
    connect(m_model, &DistanceFieldModel::error, this, &MainWindow::displayError);
}

void MainWindow::saveAs()
{
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Save distance field-enriched file"),
                                                    m_fontDir,
                                                    tr("Font files (*.ttf *.otf);;All files (*)"));
    if (!fileName.isEmpty()) {
        m_fileName = fileName;
        m_fontDir = QFileInfo(m_fileName).absolutePath();
        m_settings.setValue(QStringLiteral("fontDirectory"), m_fontDir);
        save();
    }
}


#   pragma pack(1)
struct FontDirectoryHeader
{
    quint32 sfntVersion;
    quint16 numTables;
    quint16 searchRange;
    quint16 entrySelector;
    quint16 rangeShift;
};

struct TableRecord
{
    quint32 tag;
    quint32 checkSum;
    quint32 offset;
    quint32 length;
};

struct QtdfHeader
{
    quint8 majorVersion;
    quint8 minorVersion;
    quint16 pixelSize;
    quint32 textureSize;
    quint8 flags;
    quint8 padding;
    quint32 numGlyphs;
};

struct QtdfGlyphRecord
{
    quint32 glyphIndex;
    quint32 textureOffsetX;
    quint32 textureOffsetY;
    quint32 textureWidth;
    quint32 textureHeight;
    quint32 xMargin;
    quint32 yMargin;
    qint32 boundingRectX;
    qint32 boundingRectY;
    quint32 boundingRectWidth;
    quint32 boundingRectHeight;
    quint16 textureIndex;
};

struct QtdfTextureRecord
{
    quint32 allocatedX;
    quint32 allocatedY;
    quint32 allocatedWidth;
    quint32 allocatedHeight;
    quint8 padding;
};

struct Head
{
    quint16 majorVersion;
    quint16 minorVersion;
    quint32 fontRevision;
    quint32 checkSumAdjustment;
};
#   pragma pack()

#define PAD_BUFFER(buffer, size) \
    { \
        int paddingNeed = size % 4; \
        if (paddingNeed > 0) { \
            const char padding[3] = { 0, 0, 0 }; \
            buffer.write(padding, 4 - paddingNeed); \
        } \
    }

#define ALIGN_OFFSET(offset) \
    { \
        int paddingNeed = offset % 4; \
        if (paddingNeed > 0) \
            offset += 4 - paddingNeed; \
    }

#define TO_FIXED_POINT(value) \
    ((int)(value*qreal(65536)))

void MainWindow::save()
{
    QModelIndexList list = ui->lvGlyphs->selectionModel()->selectedIndexes();
    if (list.isEmpty()) {
        QMessageBox::warning(this,
                             tr("Nothing to save"),
                             tr("No glyphs selected for saving."),
                             QMessageBox::Ok);
        return;
    }

    if (m_fileName.isEmpty()) {
        saveAs();
        return;
    }

    QFile inFile(m_fontFile);
    if (!inFile.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this,
                             tr("Can't read original font"),
                             tr("Cannot open '%s' for reading. The original font file must remain in place until the new file has been saved.").arg(m_fontFile),
                             QMessageBox::Ok);
        return;
    }

    QByteArray output;
    quint32 headOffset = 0;

    {
        QBuffer outBuffer(&output);
        outBuffer.open(QIODevice::WriteOnly);

        uchar *inData = inFile.map(0, inFile.size());
        if (inData == nullptr) {
            QMessageBox::warning(this,
                                 tr("Can't map input file"),
                                 tr("Unable to memory map input file '%s'.").arg(m_fontFile));
            return;
        }

        uchar *end = inData + inFile.size();
        if (inData + sizeof(FontDirectoryHeader) > end) {
            QMessageBox::warning(this,
                                 tr("Can't read font directory"),
                                 tr("Input file seems to be invalid or corrupt."),
                                 QMessageBox::Ok);
            return;
        }

        FontDirectoryHeader fontDirectoryHeader;
        memcpy(&fontDirectoryHeader, inData, sizeof(FontDirectoryHeader));
        quint16 numTables = qFromBigEndian(fontDirectoryHeader.numTables) + 1;
        fontDirectoryHeader.numTables = qToBigEndian(numTables);
        {
            quint16 searchRange = qFromBigEndian(fontDirectoryHeader.searchRange);
            if (searchRange / 16 < numTables) {
                quint16 pot = (searchRange / 16) * 2;
                searchRange = pot * 16;
                fontDirectoryHeader.searchRange = qToBigEndian(searchRange);
                fontDirectoryHeader.rangeShift = qToBigEndian(numTables * 16 - searchRange);

                quint16 entrySelector = 0;
                while (pot > 1) {
                    pot >>= 1;
                    entrySelector++;
                }
                fontDirectoryHeader.entrySelector = qToBigEndian(entrySelector);
            }
        }

        outBuffer.write(reinterpret_cast<char *>(&fontDirectoryHeader),
                        sizeof(FontDirectoryHeader));

        QVarLengthArray<QPair<quint32, quint32>> offsetLengthPairs;
        offsetLengthPairs.reserve(numTables - 1);

        // Copy the offset table, updating offsets
        TableRecord *offsetTable = reinterpret_cast<TableRecord *>(inData + sizeof(FontDirectoryHeader));
        quint32 currentOffset = sizeof(FontDirectoryHeader) + sizeof(TableRecord) * numTables;
        for (int i = 0; i < numTables - 1; ++i) {
            ALIGN_OFFSET(currentOffset)

            quint32 originalOffset = qFromBigEndian(offsetTable->offset);
            quint32 length = qFromBigEndian(offsetTable->length);
            offsetLengthPairs.append(qMakePair(originalOffset, length));
            if (offsetTable->tag == qToBigEndian(MAKE_TAG('h', 'e', 'a', 'd')))
                headOffset = currentOffset;

            TableRecord newTableRecord;
            memcpy(&newTableRecord, offsetTable, sizeof(TableRecord));
            newTableRecord.offset = qToBigEndian(currentOffset);
            outBuffer.write(reinterpret_cast<char *>(&newTableRecord), sizeof(TableRecord));

            offsetTable++;
            currentOffset += length;
        }

        if (headOffset == 0) {
            QMessageBox::warning(this,
                                 tr("Invalid font file"),
                                 tr("Font file does not have 'head' table."),
                                 QMessageBox::Ok);
            return;
        }

        QByteArray qtdf = createSfntTable();
        if (qtdf.isEmpty())
            return;

        {
            ALIGN_OFFSET(currentOffset)

            TableRecord qtdfRecord;
            qtdfRecord.offset = qToBigEndian(currentOffset);
            qtdfRecord.length = qToBigEndian(qtdf.length());
            qtdfRecord.tag = qToBigEndian(MAKE_TAG('q', 't', 'd', 'f'));
            quint32 checkSum = 0;
            const quint32 *start = reinterpret_cast<const quint32 *>(qtdf.constData());
            const quint32 *end = reinterpret_cast<const quint32 *>(qtdf.constData() + qtdf.length());
            while (start < end)
                checkSum += *(start++);
            qtdfRecord.checkSum = qToBigEndian(checkSum);

            outBuffer.write(reinterpret_cast<char *>(&qtdfRecord),
                            sizeof(TableRecord));
        }

        // Copy all font tables
        for (const QPair<quint32, quint32> &offsetLengthPair : offsetLengthPairs) {
            PAD_BUFFER(outBuffer, output.size())
            outBuffer.write(reinterpret_cast<char *>(inData + offsetLengthPair.first),
                            offsetLengthPair.second);
        }

        PAD_BUFFER(outBuffer, output.size())
        outBuffer.write(qtdf);
    }

    // Clear 'head' checksum and calculate new check sum adjustment
    Head *head = reinterpret_cast<Head *>(output.data() + headOffset);
    head->checkSumAdjustment = 0;

    quint32 checkSum = 0;
    const quint32 *start = reinterpret_cast<const quint32 *>(output.constData());
    const quint32 *end = reinterpret_cast<const quint32 *>(output.constData() + output.length());
    while (start < end)
        checkSum += *(start++);

    head->checkSumAdjustment = qToBigEndian(0xB1B0AFBA - checkSum);

    QFile outFile(m_fileName);
    if (!outFile.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this,
                             tr("Can't write to file"),
                             tr("Cannot open the file '%s' for writing").arg(m_fileName),
                             QMessageBox::Ok);
        return;
    }

    outFile.write(output);
}

QByteArray MainWindow::createSfntTable()
{
    QModelIndexList list = ui->lvGlyphs->selectionModel()->selectedIndexes();
    Q_ASSERT(!list.isEmpty());

    QByteArray ret;
    {
        QBuffer buffer(&ret);
        buffer.open(QIODevice::WriteOnly);

        QtdfHeader header;
        header.majorVersion = 5;
        header.minorVersion = 12;
        header.pixelSize = qToBigEndian(quint16(qRound(m_model->pixelSize())));

        const quint8 padding = 2;
        qreal scaleFactor = qreal(1) / QT_DISTANCEFIELD_SCALE(m_model->doubleGlyphResolution());
        const int radius = QT_DISTANCEFIELD_RADIUS(m_model->doubleGlyphResolution())
                / QT_DISTANCEFIELD_SCALE(m_model->doubleGlyphResolution());

        quint32 textureSize = ui->sbMaximumTextureSize->value();

        // Since we are using a single area allocator that spans all textures, we need
        // to split the textures one row before the actual maximum size, otherwise
        // glyphs that fall on the edge between two textures will expand the texture
        // they are assigned to, and this will end up being larger than the max.
        textureSize -= quint32(qCeil(m_model->pixelSize() * scaleFactor) + radius * 2 + padding * 2);
        header.textureSize = qToBigEndian(textureSize);

        header.padding = padding;
        header.flags = m_model->doubleGlyphResolution() ? 1 : 0;
        header.numGlyphs = qToBigEndian(quint32(list.size()));
        buffer.write(reinterpret_cast<char *>(&header),
                     sizeof(QtdfHeader));

        // Maximum height allocator to find optimal number of textures
        QVector<QRect> allocatedAreaPerTexture;

        struct GlyphData {
            QSGDistanceFieldGlyphCache::TexCoord texCoord;
            QRectF boundingRect;
            QSize glyphSize;
            int textureIndex;
        };
        QVector<GlyphData> glyphDatas;
        glyphDatas.resize(m_model->rowCount());

        int textureCount = 0;

        {
            QTransform scaleDown;
            scaleDown.scale(scaleFactor, scaleFactor);

            {
                bool foundOptimalSize = false;
                while (!foundOptimalSize) {
                    allocatedAreaPerTexture.clear();

                    QSGAreaAllocator allocator(QSize(textureSize, textureSize * (++textureCount)));

                    int i;
                    for (i = 0; i < list.size(); ++i) {
                        int glyphIndex = list.at(i).row();
                        GlyphData &glyphData = glyphDatas[glyphIndex];

                        QPainterPath path = m_model->path(glyphIndex);
                        glyphData.boundingRect = scaleDown.mapRect(path.boundingRect());
                        int glyphWidth = qCeil(glyphData.boundingRect.width()) + radius * 2;
                        int glyphHeight = qCeil(glyphData.boundingRect.height()) + radius * 2;

                        glyphData.glyphSize = QSize(glyphWidth + padding * 2, glyphHeight + padding * 2);

                        if (glyphData.glyphSize.width() > qint32(textureSize)
                                || glyphData.glyphSize.height() > qint32(textureSize)) {
                            QMessageBox::warning(this,
                                                 tr("Glyph too large for texture"),
                                                 tr("Glyph %1 is too large to fit in texture of size %2.")
                                                 .arg(glyphIndex).arg(textureSize));
                            return QByteArray();
                        }

                        QRect rect = allocator.allocate(glyphData.glyphSize);
                        if (rect.isNull())
                            break;

                        glyphData.textureIndex = rect.y() / textureSize;
                        while (glyphData.textureIndex >= allocatedAreaPerTexture.size())
                            allocatedAreaPerTexture.append(QRect(0, 0, 1, 1));

                        allocatedAreaPerTexture[glyphData.textureIndex] |= QRect(rect.x(),
                                                            rect.y() % textureSize,
                                                            rect.width(),
                                                            rect.height());

                        glyphData.texCoord.xMargin = QT_DISTANCEFIELD_RADIUS(m_model->doubleGlyphResolution()) / qreal(QT_DISTANCEFIELD_SCALE(m_model->doubleGlyphResolution()));
                        glyphData.texCoord.yMargin = QT_DISTANCEFIELD_RADIUS(m_model->doubleGlyphResolution()) / qreal(QT_DISTANCEFIELD_SCALE(m_model->doubleGlyphResolution()));
                        glyphData.texCoord.x = rect.x() + padding;
                        glyphData.texCoord.y = rect.y() % textureSize + padding;
                        glyphData.texCoord.width = glyphData.boundingRect.width();
                        glyphData.texCoord.height = glyphData.boundingRect.height();

                        glyphDatas.append(glyphData);
                    }

                    foundOptimalSize = i == list.size();
                    if (foundOptimalSize)
                        buffer.write(allocator.serialize());
                }
            }
        }

        QVector<QDistanceField> textures;
        textures.resize(textureCount);

        for (int textureIndex = 0; textureIndex < textureCount; ++textureIndex) {
            textures[textureIndex] = QDistanceField(allocatedAreaPerTexture.at(textureIndex).width(),
                                                    allocatedAreaPerTexture.at(textureIndex).height());

            QRect rect = allocatedAreaPerTexture.at(textureIndex);

            QtdfTextureRecord record;
            record.allocatedX = qToBigEndian(rect.x());
            record.allocatedY = qToBigEndian(rect.y());
            record.allocatedWidth = qToBigEndian(rect.width());
            record.allocatedHeight = qToBigEndian(rect.height());
            record.padding = padding;
            buffer.write(reinterpret_cast<char *>(&record),
                         sizeof(QtdfTextureRecord));
        }

        {
            for (int i = 0; i < list.size(); ++i) {
                int glyphIndex = list.at(i).row();
                QImage image = m_model->distanceField(glyphIndex);

                const GlyphData &glyphData = glyphDatas.at(glyphIndex);

                QtdfGlyphRecord glyphRecord;
                glyphRecord.glyphIndex = qToBigEndian(glyphIndex);
                glyphRecord.textureOffsetX = qToBigEndian(TO_FIXED_POINT(glyphData.texCoord.x));
                glyphRecord.textureOffsetY = qToBigEndian(TO_FIXED_POINT(glyphData.texCoord.y));
                glyphRecord.textureWidth = qToBigEndian(TO_FIXED_POINT(glyphData.texCoord.width));
                glyphRecord.textureHeight = qToBigEndian(TO_FIXED_POINT(glyphData.texCoord.height));
                glyphRecord.xMargin = qToBigEndian(TO_FIXED_POINT(glyphData.texCoord.xMargin));
                glyphRecord.yMargin = qToBigEndian(TO_FIXED_POINT(glyphData.texCoord.yMargin));
                glyphRecord.boundingRectX = qToBigEndian(TO_FIXED_POINT(glyphData.boundingRect.x()));
                glyphRecord.boundingRectY = qToBigEndian(TO_FIXED_POINT(glyphData.boundingRect.y()));
                glyphRecord.boundingRectWidth = qToBigEndian(TO_FIXED_POINT(glyphData.boundingRect.width()));
                glyphRecord.boundingRectHeight = qToBigEndian(TO_FIXED_POINT(glyphData.boundingRect.height()));
                glyphRecord.textureIndex = qToBigEndian(quint16(glyphData.textureIndex));
                buffer.write(reinterpret_cast<char *>(&glyphRecord), sizeof(QtdfGlyphRecord));

                int expectedWidth = qCeil(glyphData.texCoord.width + glyphData.texCoord.xMargin * 2);
                image = image.copy(-padding, -padding,
                                   expectedWidth + padding  * 2,
                                   image.height() + padding * 2);

                uchar *inBits = image.scanLine(0);
                uchar *outBits = textures[glyphData.textureIndex].scanLine(int(glyphData.texCoord.y) - padding)
                                    + int(glyphData.texCoord.x) - padding;
                for (int y = 0; y < image.height(); ++y) {
                    memcpy(outBits, inBits, image.width());
                    inBits += image.bytesPerLine();
                    outBits += textures[glyphData.textureIndex].width();
                }
            }
        }

        for (int i = 0; i < textures.size(); ++i) {
            const QDistanceField &texture = textures.at(i);
            const QRect &allocatedArea = allocatedAreaPerTexture.at(i);
            buffer.write(reinterpret_cast<const char *>(texture.constBits()),
                       allocatedArea.width() * allocatedArea.height());
        }

        PAD_BUFFER(buffer, ret.size())
    }

    return ret;
}

void MainWindow::writeFile()
{
    Q_ASSERT(!m_fileName.isEmpty());

    QFile file(m_fileName);
    if (file.open(QIODevice::WriteOnly)) {

    } else {
        QMessageBox::warning(this,
                             tr("Can't open file for writing"),
                             tr("Unable to open file '%1' for writing").arg(m_fileName),
                             QMessageBox::Ok);
    }
}

void MainWindow::openFont()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open font file"),
                                                    m_fontDir,
                                                    tr("Fonts (*.ttf *.otf);;All files (*)"));
    if (!fileName.isEmpty())
        open(fileName);
}

void MainWindow::updateProgressBar()
{
    m_statusBarProgressBar->setValue(m_statusBarProgressBar->value() + 1);
    updateSelection();
}

void MainWindow::startProgressBar(quint16 glyphCount)
{
    ui->action_Open->setDisabled(false);
    m_statusBarLabel->setText(tr("Generating"));
    m_statusBarProgressBar->setMaximum(glyphCount);
    m_statusBarProgressBar->setMinimum(0);
    m_statusBarProgressBar->setValue(0);
    m_statusBarProgressBar->setVisible(true);
}

void MainWindow::stopProgressBar()
{
    m_statusBarLabel->setText(tr("Ready"));
    m_statusBarProgressBar->setVisible(false);
}

void MainWindow::selectAll()
{
    QModelIndexList list = ui->lvGlyphs->selectionModel()->selectedIndexes();
    if (list.size() == ui->lvGlyphs->model()->rowCount())
        ui->lvGlyphs->clearSelection();
    else
        ui->lvGlyphs->selectAll();
}

void MainWindow::updateSelection()
{
    QModelIndexList list = ui->lvGlyphs->selectionModel()->selectedIndexes();
    QString label;
    if (list.size() == ui->lvGlyphs->model()->rowCount())
        label = tr("Deselect &All");
    else
        label = tr("Select &All");

    ui->tbSelectAll->setText(label);
    ui->actionSelect_all->setText(label);

    if (m_model != nullptr && ui->lwUnicodeRanges->count() > 0) {
        // Ignore selection changes until we are done
        disconnect(ui->lwUnicodeRanges, &QListWidget::itemSelectionChanged, this, &MainWindow::updateUnicodeRanges);

        QSet<int> selectedGlyphIndexes;
        for (const QModelIndex &modelIndex : list)
            selectedGlyphIndexes.insert(modelIndex.row());

        QList<DistanceFieldModel::UnicodeRange> unicodeRanges = m_model->unicodeRanges();
        std::sort(unicodeRanges.begin(), unicodeRanges.end());

        Q_ASSERT(ui->lwUnicodeRanges->count() == unicodeRanges.size());
        for (int i = 0; i < unicodeRanges.size(); ++i) {
            DistanceFieldModel::UnicodeRange unicodeRange = unicodeRanges.at(i);
            QListWidgetItem *item = ui->lwUnicodeRanges->item(i);

            QList<glyph_t> glyphIndexes = m_model->glyphIndexesForUnicodeRange(unicodeRange);
            Q_ASSERT(!glyphIndexes.isEmpty());

            item->setSelected(true);
            for (glyph_t glyphIndex : glyphIndexes) {
                if (!selectedGlyphIndexes.contains(glyphIndex)) {
                    item->setSelected(false);
                    break;
                }
            }
        }

        connect(ui->lwUnicodeRanges, &QListWidget::itemSelectionChanged, this, &MainWindow::updateUnicodeRanges);
    }
}

void MainWindow::updateUnicodeRanges()
{
    if (m_model == nullptr)
        return;

    disconnect(ui->lvGlyphs->selectionModel(),
               &QItemSelectionModel::selectionChanged,
               this,
               &MainWindow::updateSelection);

    QItemSelection selectedItems;

    for (int i = 0; i < ui->lwUnicodeRanges->count(); ++i) {
        QListWidgetItem *item = ui->lwUnicodeRanges->item(i);
        if (item->isSelected()) {
            DistanceFieldModel::UnicodeRange unicodeRange = item->data(Qt::UserRole).value<DistanceFieldModel::UnicodeRange>();
            QList<glyph_t> glyphIndexes = m_model->glyphIndexesForUnicodeRange(unicodeRange);

            for (glyph_t glyphIndex : glyphIndexes) {
                QModelIndex index = m_model->index(glyphIndex);
                selectedItems.select(index, index);
            }
        }
    }

    ui->lvGlyphs->selectionModel()->clearSelection();
    if (!selectedItems.isEmpty())
        ui->lvGlyphs->selectionModel()->select(selectedItems, QItemSelectionModel::Select);

    connect(ui->lvGlyphs->selectionModel(),
            &QItemSelectionModel::selectionChanged,
            this,
            &MainWindow::updateSelection);
}

void MainWindow::populateUnicodeRanges()
{
    QList<DistanceFieldModel::UnicodeRange> unicodeRanges = m_model->unicodeRanges();
    std::sort(unicodeRanges.begin(), unicodeRanges.end());

    for (DistanceFieldModel::UnicodeRange unicodeRange : unicodeRanges) {
        QString name = m_model->nameForUnicodeRange(unicodeRange);
        QListWidgetItem *item = new QListWidgetItem(name, ui->lwUnicodeRanges);
        item->setData(Qt::UserRole, unicodeRange);
    }

    ui->lwUnicodeRanges->setDisabled(false);
    ui->action_Save->setDisabled(false);
    ui->action_Save_as->setDisabled(false);
    ui->tbSave->setDisabled(false);
}

void MainWindow::displayError(const QString &errorString)
{
    QMessageBox::warning(this, tr("Error when parsing font file"), errorString, QMessageBox::Ok);
}

void MainWindow::selectString()
{
    QString s = QInputDialog::getText(this,
                                      tr("Select glyphs for string"),
                                      tr("String to parse:"));
    if (!s.isEmpty()) {
        QVector<uint> ucs4String = s.toUcs4();
        for (uint ucs4 : ucs4String) {
            glyph_t glyph = m_model->glyphIndexForUcs4(ucs4);
            if (glyph != 0) {
                ui->lvGlyphs->selectionModel()->select(m_model->index(glyph),
                                                       QItemSelectionModel::Select);
            }
        }
    }
}

void MainWindow::about()
{
    QMessageBox *msgBox = new QMessageBox(this);
    msgBox->setAttribute(Qt::WA_DeleteOnClose);
    msgBox->setWindowTitle(tr("About Qt Distance Field Generator"));
    msgBox->setText(tr("<h3>Qt Distance Field Generator</h3>"
                       "<p>Version %1.<br/>"
                       "The Qt Distance Field Generator tool allows "
                       "to prepare a font cache for Qt applications.</p>"
                       "<p>Copyright (C) %2 The Qt Company Ltd.</p>")
                    .arg(QLatin1String(QT_VERSION_STR))
                    .arg(QLatin1String("2019")));
    msgBox->show();
}

QT_END_NAMESPACE
