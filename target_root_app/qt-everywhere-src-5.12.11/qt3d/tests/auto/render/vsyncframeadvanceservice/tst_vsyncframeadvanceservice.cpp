/****************************************************************************
**
** Copyright (C) 2015 Paul Lemire
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt3D module of the Qt Toolkit.
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

#include <QtTest/QtTest>

#include <Qt3DRender/private/vsyncframeadvanceservice_p.h>

class FakeRenderThread final : public QThread
{
public:
    FakeRenderThread(Qt3DRender::Render::VSyncFrameAdvanceService *tickService)
        : m_tickService(tickService)
        , m_running(1)
        , m_submitCount(0)
    {
    }

    int submitCount() const { return m_submitCount; }

    void stopRunning()
    {
        m_running.fetchAndStoreOrdered(0);
        m_submitSemaphore.release(1);
    }

    void enqueueRenderView()
    {
        m_submitSemaphore.release(1);
    }

protected:
    // QThread interface
    void run() final
    {
        m_tickService->proceedToNextFrame();

        while (true) {
            if (!isReadyToSubmit())
                break;
            ++m_submitCount;
            m_tickService->proceedToNextFrame();
        }
    }

private:
    bool isReadyToSubmit()
    {
        m_submitSemaphore.acquire(1);
        return m_running.load() == 1;
    }

    Qt3DRender::Render::VSyncFrameAdvanceService *m_tickService;
    QAtomicInt m_running;
    QSemaphore m_submitSemaphore;
    int m_submitCount;
};

class tst_VSyncFrameAdvanceService : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void checkSynchronisation()
    {
        // GIVEN
        Qt3DRender::Render::VSyncFrameAdvanceService tickService(true);
        FakeRenderThread renderThread(&tickService);

        // WHEN
        renderThread.start();

        for (int i = 0; i < 10; ++i) {
            tickService.waitForNextFrame();
            renderThread.enqueueRenderView();
        }

        tickService.waitForNextFrame();

        renderThread.stopRunning();
        renderThread.wait();

        // THEN
        QCOMPARE(renderThread.submitCount(), 10);
    }
};

QTEST_MAIN(tst_VSyncFrameAdvanceService)

#include "tst_vsyncframeadvanceservice.moc"
