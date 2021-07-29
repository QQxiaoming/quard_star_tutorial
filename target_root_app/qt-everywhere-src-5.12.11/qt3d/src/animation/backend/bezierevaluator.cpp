/****************************************************************************
**
** Copyright (C) 2017 Klaralvdalens Datakonsult AB (KDAB).
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt3D module of the Qt Toolkit.
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

#include "bezierevaluator_p.h"
#include <private/keyframe_p.h>
#include <QtCore/qglobal.h>
#include <QtCore/qdebug.h>

#include <cmath>

QT_BEGIN_NAMESPACE

namespace {

inline double qCbrt(double x)
{
    // Android is just broken and doesn't define cbrt in std namespace
#if defined(Q_OS_ANDROID)
    if (x > 0.0)
        return std::pow(x, 1.0 / 3.0);
    else if (x < 0.0)
        return -std::pow(-x, 1.0 / 3.0);
    else
        return 0.0;
#else
    return std::cbrt(x);
#endif
}

} // anonymous

namespace Qt3DAnimation {
namespace Animation {

/*!
    \internal

    Evaluates the value of the cubic bezier at time \a time.
    This requires first finding the value of the bezier parameter, u,
    corresponding to the requested time which should itself be
    sandwiched by the provided times and keyframes.

    Once u is found, substitute this back into the cubic Bezier
    equation using the y components of the keyframe control points.
 */
float BezierEvaluator::valueForTime(float time) const
{
    const float u = parameterForTime(time);

    // Calculate powers of u and (1-u) that we need
    const float u2 = u * u;
    const float u3 = u2 * u;
    const float mu = 1.0f - u;
    const float mu2 = mu * mu;
    const float mu3 = mu2 * mu;

    // The cubic Bezier control points
    const float p0 = m_keyframe0.value;
    const float p1 = m_keyframe0.rightControlPoint.y();
    const float p2 = m_keyframe1.leftControlPoint.y();
    const float p3 = m_keyframe1.value;

    // Evaluate the cubic Bezier function
    return p0 * mu3 + 3.0f * p1 * mu2 * u + 3.0f * p2 * mu * u2 + p3 * u3;
}

/*!
    \internal

    Calculates the value of the Bezier parameter, u, for the
    requested time which is the x coordinate of the Keyframes.

    Given 4 ordered control points p0, p1, p2, and p3, the cubic
    Bezier equation is:

        x(u) = (1-u)^3 p0 + 3 (1-u)^2 u p1 + 3 (1-u) u^2 p2 + u^3 p3

    To find the value of u that corresponds with a given x
    value (time in the case of keyframes), we can expand the
    above equation, and then collect terms to arrive at:

        0 = a u^3 + b u^2 + c u + d

    where

        a = p3 - p0 + 3 (p1 - p2)
        b = 3 (p0 - 2 p1 + p2)
        c = 3 (p1 - p0)
        d = p0 - x(u)

    We can then use findCubicRoots to locate the single root of
    this cubic equation found in the range [0,1] used for this
    section of the FCurve. This works because the FCurve ensures
    that the function it represents via the Bezier control points
    in the Keyframes is single valued. (as a function of time).
    Time, therefore must be single valued on the interval and
    therefore have a single root for any given time in the interval
    covered by the Keyframes.
 */
float BezierEvaluator::parameterForTime(float time) const
{
    Q_ASSERT(time >= m_time0);
    Q_ASSERT(time <= m_time1);

    const float p0 = m_time0;
    const float p1 = m_keyframe0.rightControlPoint.x();
    const float p2 = m_keyframe1.leftControlPoint.x();
    const float p3 = m_time1;

    const float coeffs[4] = {
        p0 - time,                      // d
        3.0f * (p1 - p0),               // c
        3.0f * (p0 - 2.0f * p1 + p2),   // b
        p3 - p0 + 3.0f * (p1 - p2)      // a
    };

    float roots[3];
    const int numberOfRoots = findCubicRoots(coeffs, roots);
    for (int i = 0; i < numberOfRoots; ++i) {
        if (roots[i] >= -0.01f && roots[i] <= 1.01f)
            return qMin(qMax(roots[i], 0.0f), 1.0f);
    }

    qWarning() << "Failed to find root of cubic bezier at time" << time
               << "with coeffs: a =" << coeffs[3] << "b =" << coeffs[2]
               << "c =" << coeffs[1] << "d =" << coeffs[0];
    return 0.0f;
}

bool almostZero(float value, float threshold=1e-3f)
{
    // 1e-3 might seem excessively fuzzy, but any smaller value will make the
    // factors a, b, and c large enough to knock out the cubic solver.
    return value > -threshold && value < threshold;
}

/*!
    \internal

    Finds the roots of the cubic equation ax^3 + bx^2 + cx + d = 0 for
    real coefficients and returns the number of roots. The roots are
    put into the \a roots array. The coefficients should be passed in
    as coeffs[0] = d, coeffs[1] = c, coeffs[2] = b, coeffs[3] = a.
 */
int BezierEvaluator::findCubicRoots(const float coeffs[4], float roots[3])
{
    const float a = coeffs[3];
    const float b = coeffs[2];
    const float c = coeffs[1];
    const float d = coeffs[0];

    // Simple cases with linear, quadratic or invalid equations
    if (almostZero(a)) {
        if (almostZero(b)) {
            if (almostZero(c))
                return 0;

            roots[0] = -d / c;
            return 1;
        }
        const float discriminant = c * c - 4.f * b * d;
        if (discriminant < 0.f)
            return 0;

        if (discriminant == 0.f) {
            roots[0] = -c / (2.f * b);
            return 1;
        }

        roots[0] = (-c + std::sqrt(discriminant)) / (2.f * b);
        roots[1] = (-c - std::sqrt(discriminant)) / (2.f * b);
        return 2;
    }

    // See https://en.wikipedia.org/wiki/Cubic_function#General_solution_to_the_cubic_equation_with_real_coefficients
    // for a description. We depress the general cubic to a form that can more easily be solved. Solve it and then
    // substitue the results back to get the roots of the original cubic.
    int numberOfRoots = 0;
    const double oneThird = 1.0 / 3.0;
    const double piByThree = M_PI / 3.0;

    // Put cubic into normal format: x^3 + Ax^2 + Bx + C = 0
    const double A = double(b / a);
    const double B = double(c / a);
    const double C = double(d / a);

    // Substitute x = y - A/3 to eliminate quadratic term (depressed form):
    // x^3 + px + q = 0
    const double Asq = A * A;
    const double p = oneThird * (-oneThird * Asq + B);
    const double q = 1.0 / 2.0 * (2.0 / 27.0 * A * Asq - oneThird * A * B + C);

    // Use Cardano's formula
    const double pCubed = p * p * p;
    const double discriminant = q * q + pCubed;

    if (almostZero(discriminant, 1e-6f)) {
        if (qIsNull(q)) {
            // One repeated triple root
            roots[0] = 0.0;
            numberOfRoots = 1;
        } else {
            // One single and one double root
            double u = qCbrt(-q);
            roots[0] = 2.0 * u;
            roots[1] = -u;
            numberOfRoots = 2;
        }
    } else if (discriminant < 0) {
        // Three real solutions
        double phi = oneThird * std::acos(-q / std::sqrt(-pCubed));
        double t = 2.0 * std::sqrt(-p);

        roots[0] =  t * std::cos(phi);
        roots[1] = -t * std::cos(phi + piByThree);
        roots[2] = -t * std::cos(phi - piByThree);
        numberOfRoots = 3;
    } else {
        // One real solution
        double sqrtDisc = std::sqrt(discriminant);
        double u = qCbrt(sqrtDisc - q);
        double v = -qCbrt(sqrtDisc + q);

        roots[0] = u + v;
        numberOfRoots = 1;
    }

    // Substitute back in
    const double sub = oneThird * A;
    for (int i = 0; i < numberOfRoots; ++i) {
        roots[i] -= sub;
        // Take care of cases where we are close to zero or one
        if (almostZero(roots[i], 1e-6f))
            roots[i] = 0.f;
        if (almostZero(roots[i] - 1.f, 1e-6f))
            roots[i] = 1.f;
    }

    return numberOfRoots;
}

} // namespace Animation
} // namespace Qt3DAnimation

QT_END_NAMESPACE
