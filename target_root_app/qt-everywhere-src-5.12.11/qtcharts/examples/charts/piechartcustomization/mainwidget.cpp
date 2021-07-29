/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Charts module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL$
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
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/
#include "mainwidget.h"
#include "customslice.h"
#include "pentool.h"
#include "brushtool.h"
#include <QtWidgets/QPushButton>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QFontDialog>
#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>

QT_CHARTS_USE_NAMESPACE

MainWidget::MainWidget(QWidget *parent)
    : QWidget(parent),
      m_slice(0)
{
    // create chart
    QChart *chart = new QChart;
    chart->setTitle("Piechart customization");
    chart->setAnimationOptions(QChart::AllAnimations);

    // create series
    m_series = new QPieSeries();
    *m_series << new CustomSlice("Slice 1", 10.0);
    *m_series << new CustomSlice("Slice 2", 20.0);
    *m_series << new CustomSlice("Slice 3", 30.0);
    *m_series << new CustomSlice("Slice 4", 40.0);
    *m_series << new CustomSlice("Slice 5", 50.0);
    m_series->setLabelsVisible();
    chart->addSeries(m_series);

    connect(m_series, &QPieSeries::clicked, this, &MainWidget::handleSliceClicked);

    // chart settings
    m_themeComboBox = new QComboBox();
    m_themeComboBox->addItem("Light", QChart::ChartThemeLight);
    m_themeComboBox->addItem("BlueCerulean", QChart::ChartThemeBlueCerulean);
    m_themeComboBox->addItem("Dark", QChart::ChartThemeDark);
    m_themeComboBox->addItem("BrownSand", QChart::ChartThemeBrownSand);
    m_themeComboBox->addItem("BlueNcs", QChart::ChartThemeBlueNcs);
    m_themeComboBox->addItem("High Contrast", QChart::ChartThemeHighContrast);
    m_themeComboBox->addItem("Blue Icy", QChart::ChartThemeBlueIcy);
    m_themeComboBox->addItem("Qt", QChart::ChartThemeQt);

    m_aaCheckBox = new QCheckBox();
    m_animationsCheckBox = new QCheckBox();
    m_animationsCheckBox->setCheckState(Qt::Checked);

    m_legendCheckBox = new QCheckBox();

    QFormLayout *chartSettingsLayout = new QFormLayout();
    chartSettingsLayout->addRow("Theme", m_themeComboBox);
    chartSettingsLayout->addRow("Antialiasing", m_aaCheckBox);
    chartSettingsLayout->addRow("Animations", m_animationsCheckBox);
    chartSettingsLayout->addRow("Legend", m_legendCheckBox);
    QGroupBox *chartSettings = new QGroupBox("Chart");
    chartSettings->setLayout(chartSettingsLayout);

    connect(m_themeComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &MainWidget::updateChartSettings);
    connect(m_aaCheckBox, &QCheckBox::toggled, this, &MainWidget::updateChartSettings);
    connect(m_animationsCheckBox, &QCheckBox::toggled, this, &MainWidget::updateChartSettings);
    connect(m_legendCheckBox, &QCheckBox::toggled, this, &MainWidget::updateChartSettings);

    // series settings
    m_hPosition = new QDoubleSpinBox();
    m_hPosition->setMinimum(0.0);
    m_hPosition->setMaximum(1.0);
    m_hPosition->setSingleStep(0.1);
    m_hPosition->setValue(m_series->horizontalPosition());

    m_vPosition = new QDoubleSpinBox();
    m_vPosition->setMinimum(0.0);
    m_vPosition->setMaximum(1.0);
    m_vPosition->setSingleStep(0.1);
    m_vPosition->setValue(m_series->verticalPosition());

    m_sizeFactor = new QDoubleSpinBox();
    m_sizeFactor->setMinimum(0.0);
    m_sizeFactor->setMaximum(1.0);
    m_sizeFactor->setSingleStep(0.1);
    m_sizeFactor->setValue(m_series->pieSize());

    m_startAngle = new QDoubleSpinBox();
    m_startAngle->setMinimum(-720);
    m_startAngle->setMaximum(720);
    m_startAngle->setValue(m_series->pieStartAngle());
    m_startAngle->setSingleStep(1);

    m_endAngle = new QDoubleSpinBox();
    m_endAngle->setMinimum(-720);
    m_endAngle->setMaximum(720);
    m_endAngle->setValue(m_series->pieEndAngle());
    m_endAngle->setSingleStep(1);

    m_holeSize = new QDoubleSpinBox();
    m_holeSize->setMinimum(0.0);
    m_holeSize->setMaximum(1.0);
    m_holeSize->setSingleStep(0.1);
    m_holeSize->setValue(m_series->holeSize());

    QPushButton *appendSlice = new QPushButton("Append slice");
    QPushButton *insertSlice = new QPushButton("Insert slice");
    QPushButton *removeSlice = new QPushButton("Remove selected slice");

    QFormLayout *seriesSettingsLayout = new QFormLayout();
    seriesSettingsLayout->addRow("Horizontal position", m_hPosition);
    seriesSettingsLayout->addRow("Vertical position", m_vPosition);
    seriesSettingsLayout->addRow("Size factor", m_sizeFactor);
    seriesSettingsLayout->addRow("Start angle", m_startAngle);
    seriesSettingsLayout->addRow("End angle", m_endAngle);
    seriesSettingsLayout->addRow("Hole size", m_holeSize);
    seriesSettingsLayout->addRow(appendSlice);
    seriesSettingsLayout->addRow(insertSlice);
    seriesSettingsLayout->addRow(removeSlice);
    QGroupBox *seriesSettings = new QGroupBox("Series");
    seriesSettings->setLayout(seriesSettingsLayout);

    connect(m_vPosition,
            static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            this, &MainWidget::updateSerieSettings);
    connect(m_hPosition,
            static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            this, &MainWidget::updateSerieSettings);
    connect(m_sizeFactor,
            static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            this, &MainWidget::updateSerieSettings);
    connect(m_startAngle,
            static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            this, &MainWidget::updateSerieSettings);
    connect(m_endAngle,
            static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            this, &MainWidget::updateSerieSettings);
    connect(m_holeSize,
            static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            this, &MainWidget::updateSerieSettings);
    connect(appendSlice, &QPushButton::clicked, this, &MainWidget::appendSlice);
    connect(insertSlice, &QPushButton::clicked, this, &MainWidget::insertSlice);
    connect(removeSlice, &QPushButton::clicked, this, &MainWidget::removeSlice);

    // slice settings
    m_sliceName = new QLineEdit("<click a slice>");
    m_sliceName->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    m_sliceValue = new QDoubleSpinBox();
    m_sliceValue->setMaximum(1000);
    m_sliceLabelVisible = new QCheckBox();
    m_sliceLabelArmFactor = new QDoubleSpinBox();
    m_sliceLabelArmFactor->setSingleStep(0.01);
    m_sliceExploded = new QCheckBox();
    m_sliceExplodedFactor = new QDoubleSpinBox();
    m_sliceExplodedFactor->setSingleStep(0.01);
    m_pen = new QPushButton();
    m_penTool = new PenTool("Slice pen", this);
    m_brush = new QPushButton();
    m_brushTool = new BrushTool("Slice brush", this);
    m_font = new QPushButton();
    m_labelBrush = new QPushButton();
    m_labelBrushTool = new BrushTool("Label brush", this);
    m_labelPosition = new QComboBox(this);
    m_labelPosition->addItem("Outside", QPieSlice::LabelOutside);
    m_labelPosition->addItem("Inside horizontal", QPieSlice::LabelInsideHorizontal);
    m_labelPosition->addItem("Inside tangential", QPieSlice::LabelInsideTangential);
    m_labelPosition->addItem("Inside normal", QPieSlice::LabelInsideNormal);

    QFormLayout *sliceSettingsLayout = new QFormLayout();
    sliceSettingsLayout->addRow("Label", m_sliceName);
    sliceSettingsLayout->addRow("Value", m_sliceValue);
    sliceSettingsLayout->addRow("Pen", m_pen);
    sliceSettingsLayout->addRow("Brush", m_brush);
    sliceSettingsLayout->addRow("Label visible", m_sliceLabelVisible);
    sliceSettingsLayout->addRow("Label font", m_font);
    sliceSettingsLayout->addRow("Label brush", m_labelBrush);
    sliceSettingsLayout->addRow("Label position", m_labelPosition);
    sliceSettingsLayout->addRow("Label arm length", m_sliceLabelArmFactor);
    sliceSettingsLayout->addRow("Exploded", m_sliceExploded);
    sliceSettingsLayout->addRow("Explode distance", m_sliceExplodedFactor);
    QGroupBox *sliceSettings = new QGroupBox("Selected slice");
    sliceSettings->setLayout(sliceSettingsLayout);

    connect(m_sliceName, &QLineEdit::textChanged, this, &MainWidget::updateSliceSettings);
    connect(m_sliceValue,
            static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            this, &MainWidget::updateSliceSettings);
    connect(m_pen, &QPushButton::clicked, m_penTool, &PenTool::show);
    connect(m_penTool, &PenTool::changed, this, &MainWidget::updateSliceSettings);
    connect(m_brush, &QPushButton::clicked, m_brushTool, &BrushTool::show);
    connect(m_brushTool, &BrushTool::changed, this, &MainWidget::updateSliceSettings);
    connect(m_font, &QPushButton::clicked, this, &MainWidget::showFontDialog);
    connect(m_labelBrush, &QPushButton::clicked, m_labelBrushTool, &BrushTool::show);
    connect(m_labelBrushTool, &BrushTool::changed, this, &MainWidget::updateSliceSettings);
    connect(m_sliceLabelVisible, &QCheckBox::toggled, this, &MainWidget::updateSliceSettings);
    connect(m_sliceLabelVisible, &QCheckBox::toggled, this, &MainWidget::updateSliceSettings);
    connect(m_sliceLabelArmFactor,
            static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            this, &MainWidget::updateSliceSettings);
    connect(m_sliceExploded, &QCheckBox::toggled, this, &MainWidget::updateSliceSettings);
    connect(m_sliceExplodedFactor,
            static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            this, &MainWidget::updateSliceSettings);
    connect(m_labelPosition,
            static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &MainWidget::updateSliceSettings);

    // create chart view
    m_chartView = new QChartView(chart);

    // create main layout
    QVBoxLayout *settingsLayout = new QVBoxLayout();
    settingsLayout->addWidget(chartSettings);
    settingsLayout->addWidget(seriesSettings);
    settingsLayout->addWidget(sliceSettings);
    settingsLayout->addStretch();

    QGridLayout *baseLayout = new QGridLayout();
    baseLayout->addLayout(settingsLayout, 0, 0);
    baseLayout->addWidget(m_chartView, 0, 1);
    setLayout(baseLayout);

    updateSerieSettings();
    updateChartSettings();
}


void MainWidget::updateChartSettings()
{
    QChart::ChartTheme theme = static_cast<QChart::ChartTheme>(m_themeComboBox->itemData(
                m_themeComboBox->currentIndex()).toInt());
    m_chartView->chart()->setTheme(theme);
    m_chartView->setRenderHint(QPainter::Antialiasing, m_aaCheckBox->isChecked());

    if (m_animationsCheckBox->checkState() == Qt::Checked)
        m_chartView->chart()->setAnimationOptions(QChart::AllAnimations);
    else
        m_chartView->chart()->setAnimationOptions(QChart::NoAnimation);

    if (m_legendCheckBox->checkState() == Qt::Checked)
        m_chartView->chart()->legend()->show();
    else
        m_chartView->chart()->legend()->hide();
}

void MainWidget::updateSerieSettings()
{
    m_series->setHorizontalPosition(m_hPosition->value());
    m_series->setVerticalPosition(m_vPosition->value());
    m_series->setPieSize(m_sizeFactor->value());
    m_holeSize->setMaximum(m_sizeFactor->value());
    m_series->setPieStartAngle(m_startAngle->value());
    m_series->setPieEndAngle(m_endAngle->value());
    m_series->setHoleSize(m_holeSize->value());
}

void MainWidget::updateSliceSettings()
{
    if (!m_slice)
        return;

    m_slice->setLabel(m_sliceName->text());

    m_slice->setValue(m_sliceValue->value());

    m_slice->setPen(m_penTool->pen());
    m_slice->setBrush(m_brushTool->brush());

    m_slice->setLabelBrush(m_labelBrushTool->brush());
    m_slice->setLabelVisible(m_sliceLabelVisible->isChecked());
    m_slice->setLabelArmLengthFactor(m_sliceLabelArmFactor->value());
    // We assume that label position index is in sync with the enum
    m_slice->setLabelPosition((QPieSlice::LabelPosition)m_labelPosition->currentIndex());

    m_slice->setExploded(m_sliceExploded->isChecked());
    m_slice->setExplodeDistanceFactor(m_sliceExplodedFactor->value());
}

void MainWidget::handleSliceClicked(QPieSlice *slice)
{
    m_slice = static_cast<CustomSlice *>(slice);

    // name
    m_sliceName->blockSignals(true);
    m_sliceName->setText(slice->label());
    m_sliceName->blockSignals(false);

    // value
    m_sliceValue->blockSignals(true);
    m_sliceValue->setValue(slice->value());
    m_sliceValue->blockSignals(false);

    // pen
    m_pen->setText(PenTool::name(m_slice->pen()));
    m_penTool->setPen(m_slice->pen());

    // brush
    m_brush->setText(m_slice->originalBrush().color().name());
    m_brushTool->setBrush(m_slice->originalBrush());

    // label
    m_labelBrush->setText(BrushTool::name(m_slice->labelBrush()));
    m_labelBrushTool->setBrush(m_slice->labelBrush());
    m_font->setText(slice->labelFont().toString());
    m_sliceLabelVisible->blockSignals(true);
    m_sliceLabelVisible->setChecked(slice->isLabelVisible());
    m_sliceLabelVisible->blockSignals(false);
    m_sliceLabelArmFactor->blockSignals(true);
    m_sliceLabelArmFactor->setValue(slice->labelArmLengthFactor());
    m_sliceLabelArmFactor->blockSignals(false);
    m_labelPosition->blockSignals(true);
    // We assume that label position index is in sync with the enum
    m_labelPosition->setCurrentIndex(slice->labelPosition());
    m_labelPosition->blockSignals(false);

    // exploded
    m_sliceExploded->blockSignals(true);
    m_sliceExploded->setChecked(slice->isExploded());
    m_sliceExploded->blockSignals(false);
    m_sliceExplodedFactor->blockSignals(true);
    m_sliceExplodedFactor->setValue(slice->explodeDistanceFactor());
    m_sliceExplodedFactor->blockSignals(false);
}

void MainWidget::showFontDialog()
{
    if (!m_slice)
        return;

    QFontDialog dialog(m_slice->labelFont());
    dialog.show();
    dialog.exec();

    m_slice->setLabelFont(dialog.currentFont());
    m_font->setText(dialog.currentFont().toString());
}

void MainWidget::appendSlice()
{
    *m_series << new CustomSlice("Slice " + QString::number(m_series->count() + 1), 10.0);
}

void MainWidget::insertSlice()
{
    if (!m_slice)
        return;

    int i = m_series->slices().indexOf(m_slice);

    m_series->insert(i, new CustomSlice("Slice " + QString::number(m_series->count() + 1), 10.0));
}

void MainWidget::removeSlice()
{
    if (!m_slice)
        return;

    m_sliceName->setText("<click a slice>");

    m_series->remove(m_slice);
    m_slice = 0;
}

#include "moc_mainwidget.cpp"
