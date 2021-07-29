/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtQml module of the Qt Toolkit.
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

#pragma once

#include <QtCore/qglobal.h>
#include <QtQml/private/qtqmlglobal_p.h>

#include <memory>
#if __cplusplus > 201402L && QT_HAS_INCLUDE(<optional>)
#include <optional>
#else

namespace std {

struct nullopt_t {};

constexpr nullopt_t nullopt {};

template<typename T>
class optional {
public:
    optional() = default;
    optional(nullopt_t) {}
    optional(const T &v) : _value(v), _hasValue(true) {}
    ~optional() = default;

    optional &operator =(nullopt_t) {
        _value = T();
        _hasValue = false;
        return *this;
    }

    T operator->() { return _value; }
    T operator*() { return _value; }

    operator bool() const { return _hasValue; }
    bool has_value() const { return _hasValue; }

    T value() const { return _value; }

private:
    T _value = T();
    bool _hasValue = false;
};

}

#endif

#if __cplusplus < 201402L && !QT_CONFIG(cxx14_make_unique)\
&& !defined(__cpp_lib_make_unique)

namespace std {
    template<typename T, class ...Args>
    unique_ptr<T> make_unique(Args &&...args)
      { return unique_ptr<T>(new T(std::forward<Args>(args)...)); }
}

#endif
