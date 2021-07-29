/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
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

#include "audiodevices.h"

// Utility functions for converting QAudioFormat fields into text

static QString toString(QAudioFormat::SampleType sampleType)
{
    QString result("Unknown");
    switch (sampleType) {
    case QAudioFormat::SignedInt:
        result = "SignedInt";
        break;
    case QAudioFormat::UnSignedInt:
        result = "UnSignedInt";
        break;
    case QAudioFormat::Float:
        result = "Float";
        break;
    case QAudioFormat::Unknown:
        result = "Unknown";
    }
    return result;
}

static QString toString(QAudioFormat::Endian endian)
{
    QString result("Unknown");
    switch (endian) {
    case QAudioFormat::LittleEndian:
        result = "LittleEndian";
        break;
    case QAudioFormat::BigEndian:
        result = "BigEndian";
        break;
    }
    return result;
}


AudioDevicesBase::AudioDevicesBase(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi(this);
}

AudioDevicesBase::~AudioDevicesBase() {}


AudioTest::AudioTest(QWidget *parent)
    : AudioDevicesBase(parent)
{
    connect(testButton, &QPushButton::clicked, this, &AudioTest::test);
    connect(modeBox, QOverload<int>::of(&QComboBox::activated), this, &AudioTest::modeChanged);
    connect(deviceBox, QOverload<int>::of(&QComboBox::activated), this, &AudioTest::deviceChanged);
    connect(sampleRateBox, QOverload<int>::of(&QComboBox::activated), this, &AudioTest::sampleRateChanged);
    connect(channelsBox, QOverload<int>::of(&QComboBox::activated), this, &AudioTest::channelChanged);
    connect(codecsBox, QOverload<int>::of(&QComboBox::activated), this, &AudioTest::codecChanged);
    connect(sampleSizesBox, QOverload<int>::of(&QComboBox::activated), this, &AudioTest::sampleSizeChanged);
    connect(sampleTypesBox, QOverload<int>::of(&QComboBox::activated), this, &AudioTest::sampleTypeChanged);
    connect(endianBox, QOverload<int>::of(&QComboBox::activated), this, &AudioTest::endianChanged);
    connect(populateTableButton, &QPushButton::clicked, this, &AudioTest::populateTable);

    modeBox->setCurrentIndex(0);
    modeChanged(0);
    deviceBox->setCurrentIndex(0);
    deviceChanged(0);
}

void AudioTest::test()
{
    // tries to set all the settings picked.
    testResult->clear();

    if (!m_deviceInfo.isNull()) {
        if (m_deviceInfo.isFormatSupported(m_settings)) {
            testResult->setText(tr("Success"));
            nearestSampleRate->setText("");
            nearestChannel->setText("");
            nearestCodec->setText("");
            nearestSampleSize->setText("");
            nearestSampleType->setText("");
            nearestEndian->setText("");
        } else {
            QAudioFormat nearest = m_deviceInfo.nearestFormat(m_settings);
            testResult->setText(tr("Failed"));
            nearestSampleRate->setText(QString("%1").arg(nearest.sampleRate()));
            nearestChannel->setText(QString("%1").arg(nearest.channelCount()));
            nearestCodec->setText(nearest.codec());
            nearestSampleSize->setText(QString("%1").arg(nearest.sampleSize()));
            nearestSampleType->setText(toString(nearest.sampleType()));
            nearestEndian->setText(toString(nearest.byteOrder()));
        }
    }
    else
        testResult->setText(tr("No Device"));
}

void AudioTest::modeChanged(int idx)
{
    testResult->clear();
    deviceBox->clear();
    const QAudio::Mode mode = idx == 0 ? QAudio::AudioInput : QAudio::AudioOutput;
    for (auto &deviceInfo: QAudioDeviceInfo::availableDevices(mode))
        deviceBox->addItem(deviceInfo.deviceName(), qVariantFromValue(deviceInfo));

    deviceBox->setCurrentIndex(0);
    deviceChanged(0);
}

void AudioTest::deviceChanged(int idx)
{
    testResult->clear();

    if (deviceBox->count() == 0)
        return;

    // device has changed
    m_deviceInfo = deviceBox->itemData(idx).value<QAudioDeviceInfo>();

    sampleRateBox->clear();
    QList<int> sampleRatez = m_deviceInfo.supportedSampleRates();
    for (int i = 0; i < sampleRatez.size(); ++i)
        sampleRateBox->addItem(QString("%1").arg(sampleRatez.at(i)));
    if (sampleRatez.size())
        m_settings.setSampleRate(sampleRatez.at(0));

    channelsBox->clear();
    QList<int> chz = m_deviceInfo.supportedChannelCounts();
    for (int i = 0; i < chz.size(); ++i)
        channelsBox->addItem(QString("%1").arg(chz.at(i)));
    if (chz.size())
        m_settings.setChannelCount(chz.at(0));

    codecsBox->clear();
    QStringList codecs = m_deviceInfo.supportedCodecs();
    for (int i = 0; i < codecs.size(); ++i)
        codecsBox->addItem(QString("%1").arg(codecs.at(i)));
    if (codecs.size())
        m_settings.setCodec(codecs.at(0));
    // Add false to create failed condition!
    codecsBox->addItem("audio/test");

    sampleSizesBox->clear();
    QList<int> sampleSizez = m_deviceInfo.supportedSampleSizes();
    for (int i = 0; i < sampleSizez.size(); ++i)
        sampleSizesBox->addItem(QString("%1").arg(sampleSizez.at(i)));
    if (sampleSizez.size())
        m_settings.setSampleSize(sampleSizez.at(0));

    sampleTypesBox->clear();
    QList<QAudioFormat::SampleType> sampleTypez = m_deviceInfo.supportedSampleTypes();

    for (int i = 0; i < sampleTypez.size(); ++i)
        sampleTypesBox->addItem(toString(sampleTypez.at(i)));
    if (sampleTypez.size())
        m_settings.setSampleType(sampleTypez.at(0));

    endianBox->clear();
    QList<QAudioFormat::Endian> endianz = m_deviceInfo.supportedByteOrders();
    for (int i = 0; i < endianz.size(); ++i)
        endianBox->addItem(toString(endianz.at(i)));
    if (endianz.size())
        m_settings.setByteOrder(endianz.at(0));

    allFormatsTable->clearContents();
}

void AudioTest::populateTable()
{
    int row = 0;

    QAudioFormat format;
    for (auto codec: m_deviceInfo.supportedCodecs()) {
        format.setCodec(codec);
        for (auto sampleRate: m_deviceInfo.supportedSampleRates()) {
            format.setSampleRate(sampleRate);
            for (auto channels: m_deviceInfo.supportedChannelCounts()) {
                format.setChannelCount(channels);
                for (auto sampleType: m_deviceInfo.supportedSampleTypes()) {
                    format.setSampleType(sampleType);
                    for (auto sampleSize: m_deviceInfo.supportedSampleSizes()) {
                        format.setSampleSize(sampleSize);
                        for (auto endian: m_deviceInfo.supportedByteOrders()) {
                            format.setByteOrder(endian);
                            if (m_deviceInfo.isFormatSupported(format)) {
                                allFormatsTable->setRowCount(row + 1);

                                QTableWidgetItem *codecItem = new QTableWidgetItem(format.codec());
                                allFormatsTable->setItem(row, 0, codecItem);

                                QTableWidgetItem *sampleRateItem = new QTableWidgetItem(QString("%1").arg(format.sampleRate()));
                                allFormatsTable->setItem(row, 1, sampleRateItem);

                                QTableWidgetItem *channelsItem = new QTableWidgetItem(QString("%1").arg(format.channelCount()));
                                allFormatsTable->setItem(row, 2, channelsItem);

                                QTableWidgetItem *sampleTypeItem = new QTableWidgetItem(toString(format.sampleType()));
                                allFormatsTable->setItem(row, 3, sampleTypeItem);

                                QTableWidgetItem *sampleSizeItem = new QTableWidgetItem(QString("%1").arg(format.sampleSize()));
                                allFormatsTable->setItem(row, 4, sampleSizeItem);

                                QTableWidgetItem *byteOrderItem = new QTableWidgetItem(toString(format.byteOrder()));
                                allFormatsTable->setItem(row, 5, byteOrderItem);

                                ++row;
                            }
                        }
                    }
                }
            }
        }
    }
}

void AudioTest::sampleRateChanged(int idx)
{
    // sample rate has changed
    m_settings.setSampleRate(sampleRateBox->itemText(idx).toInt());
}

void AudioTest::channelChanged(int idx)
{
    m_settings.setChannelCount(channelsBox->itemText(idx).toInt());
}

void AudioTest::codecChanged(int idx)
{
    m_settings.setCodec(codecsBox->itemText(idx));
}

void AudioTest::sampleSizeChanged(int idx)
{
    m_settings.setSampleSize(sampleSizesBox->itemText(idx).toInt());
}

void AudioTest::sampleTypeChanged(int idx)
{
    switch (sampleTypesBox->itemText(idx).toInt()) {
        case QAudioFormat::SignedInt:
            m_settings.setSampleType(QAudioFormat::SignedInt);
            break;
        case QAudioFormat::UnSignedInt:
            m_settings.setSampleType(QAudioFormat::UnSignedInt);
            break;
        case QAudioFormat::Float:
            m_settings.setSampleType(QAudioFormat::Float);
    }
}

void AudioTest::endianChanged(int idx)
{
    switch (endianBox->itemText(idx).toInt()) {
        case QAudioFormat::LittleEndian:
            m_settings.setByteOrder(QAudioFormat::LittleEndian);
            break;
        case QAudioFormat::BigEndian:
            m_settings.setByteOrder(QAudioFormat::BigEndian);
    }
}
