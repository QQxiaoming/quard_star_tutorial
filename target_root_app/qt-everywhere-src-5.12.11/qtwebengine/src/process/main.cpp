/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
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

#include "process_main.h"

#include <QCoreApplication>
#include <stdio.h>
#include <memory>

#if defined(Q_OS_LINUX)

struct tm;
struct stat;
struct stat64;

// exported in sandbox/linux/services/libc_interceptor.cc
namespace sandbox {
struct tm* localtime_override(const time_t* timep);
struct tm* localtime64_override(const time_t* timep);
struct tm* localtime_r_override(const time_t* timep, struct tm* result);
struct tm* localtime64_r_override(const time_t* timep, struct tm* result);
}

// from sandbox/linux/services/libc_interceptor.cc
__attribute__ ((__visibility__("default")))
struct tm* localtime_proxy(const time_t* timep) __asm__ ("localtime");
struct tm* localtime_proxy(const time_t* timep)
{
    return sandbox::localtime_override(timep);
}

__attribute__ ((__visibility__("default")))
struct tm* localtime64_proxy(const time_t* timep) __asm__ ("localtime64");
struct tm* localtime64_proxy(const time_t* timep)
{
    return sandbox::localtime64_override(timep);
}

__attribute__ ((__visibility__("default")))
struct tm* localtime_r_proxy(const time_t* timep, struct tm* result) __asm__ ("localtime_r");
struct tm* localtime_r_proxy(const time_t* timep, struct tm* result)
{
    return sandbox::localtime_r_override(timep, result);
}

__attribute__ ((__visibility__("default")))
struct tm* localtime64_r_proxy(const time_t* timep, struct tm* result) __asm__ ("localtime64_r");
struct tm* localtime64_r_proxy(const time_t* timep, struct tm* result)
{
    return sandbox::localtime64_r_override(timep, result);
}

#endif // defined(OS_LINUX)

#ifdef Q_OS_WIN
void initDpiAwareness();
#endif // defined(Q_OS_WIN)

int main(int argc, const char **argv)
{
#ifdef Q_OS_WIN
    initDpiAwareness();
#endif

    // Chromium on Linux manipulates argv to set a process title
    // (see set_process_title_linux.cc).
    // This can interfere with QCoreApplication::applicationFilePath,
    // which assumes that argv[0] only contains the executable path.
    //
    // Avoid this by making a deep copy of argv and pass this
    // to QCoreApplication. Use a unique_ptr with custom deleter to
    // clean up on exit.

    auto dt = [](char* av[]) {
        for (char **a = av; *a; a++)
          delete[] *a;
        delete[] av;
    };

    std::unique_ptr<char*[], decltype(dt)> argv_(new char*[argc+1], dt);
    for (int i = 0; i < argc; ++i) {
        size_t len = strlen(argv[i]) + 1;
        argv_[i] = new char[len];
        strcpy(argv_[i], argv[i]);
    }
    argv_[argc] = 0;

    QCoreApplication qtApplication(argc, argv_.get());

    return QtWebEngine::processMain(argc, argv);
}

