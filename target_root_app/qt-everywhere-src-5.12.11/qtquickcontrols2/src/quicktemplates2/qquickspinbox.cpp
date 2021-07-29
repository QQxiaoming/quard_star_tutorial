/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the Qt Quick Templates 2 module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qquickspinbox_p.h"
#include "qquickcontrol_p_p.h"
#include "qquickdeferredexecute_p_p.h"

#include <QtGui/qguiapplication.h>
#include <QtGui/qstylehints.h>

#include <QtQml/qqmlinfo.h>
#include <QtQml/private/qqmllocale_p.h>
#include <QtQml/private/qqmlengine_p.h>
#include <QtQuick/private/qquicktextinput_p.h>

QT_BEGIN_NAMESPACE

// copied from qabstractbutton.cpp
static const int AUTO_REPEAT_DELAY = 300;
static const int AUTO_REPEAT_INTERVAL = 100;

/*!
    \qmltype SpinBox
    \inherits Control
    \instantiates QQuickSpinBox
    \inqmlmodule QtQuick.Controls
    \since 5.7
    \ingroup input
    \ingroup qtquickcontrols2-focusscopes
    \brief Allows the user to select from a set of preset values.

    \image qtquickcontrols2-spinbox.png

    SpinBox allows the user to choose an integer value by clicking the up
    or down indicator buttons, or by pressing up or down on the keyboard.
    Optionally, SpinBox can be also made \l editable, so the user can enter
    a text value in the input field.

    By default, SpinBox provides discrete values in the range of \c [0-99]
    with a \l stepSize of \c 1.

    \snippet qtquickcontrols2-spinbox.qml 1

    \section2 Custom Values

    \image qtquickcontrols2-spinbox-textual.png

    Even though SpinBox works on integer values, it can be customized to
    accept arbitrary input values. The following snippet demonstrates how
    \l validator, \l textFromValue and \l valueFromText can be used to
    customize the default behavior.

    \snippet qtquickcontrols2-spinbox-textual.qml 1

    In the same manner, SpinBox can be customized to accept floating point
    numbers:

    \image qtquickcontrols2-spinbox-double.png

    \snippet qtquickcontrols2-spinbox-double.qml 1

    \sa Tumbler, {Customizing SpinBox}, {Focus Management in Qt Quick Controls 2}
*/

/*!
    \since QtQuick.Controls 2.2 (Qt 5.9)
    \qmlsignal QtQuick.Controls::SpinBox::valueModified()

    This signal is emitted when the spin box value has been interactively
    modified by the user by either touch, mouse, wheel, or keys.
*/

class QQuickSpinBoxPrivate : public QQuickControlPrivate
{
    Q_DECLARE_PUBLIC(QQuickSpinBox)

public:
    int boundValue(int value, bool wrap) const;
    void updateValue();
    bool setValue(int value, bool wrap, bool modified);
    bool stepBy(int steps, bool modified);
    void increase(bool modified);
    void decrease(bool modified);

    int effectiveStepSize() const;

    void updateDisplayText();
    void setDisplayText(const QString &displayText);

    bool upEnabled() const;
    void updateUpEnabled();
    bool downEnabled() const;
    void updateDownEnabled();
    void updateHover(const QPointF &pos);

    void startRepeatDelay();
    void startPressRepeat();
    void stopPressRepeat();

    void handlePress(const QPointF &point) override;
    void handleMove(const QPointF &point) override;
    void handleRelease(const QPointF &point) override;
    void handleUngrab() override;

    void itemImplicitWidthChanged(QQuickItem *item) override;
    void itemImplicitHeightChanged(QQuickItem *item) override;

    bool editable = false;
    bool wrap = false;
    int from = 0;
    int to = 99;
    int value = 0;
    int stepSize = 1;
    int delayTimer = 0;
    int repeatTimer = 0;
    QString displayText;
    QQuickSpinButton *up = nullptr;
    QQuickSpinButton *down = nullptr;
    QValidator *validator = nullptr;
    mutable QJSValue textFromValue;
    mutable QJSValue valueFromText;
    Qt::InputMethodHints inputMethodHints = Qt::ImhDigitsOnly;
};

class QQuickSpinButtonPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QQuickSpinButton)

public:
    static QQuickSpinButtonPrivate *get(QQuickSpinButton *button)
    {
        return button->d_func();
    }

    void cancelIndicator();
    void executeIndicator(bool complete = false);

    bool pressed = false;
    bool hovered = false;
    QQuickDeferredPointer<QQuickItem> indicator;
};

int QQuickSpinBoxPrivate::boundValue(int value, bool wrap) const
{
    bool inverted = from > to;
    if (!wrap)
        return inverted ? qBound(to, value, from) : qBound(from, value, to);

    int f = inverted ? to : from;
    int t = inverted ? from : to;
    if (value < f)
        value = t;
    else if (value > t)
        value = f;

    return value;
}

void QQuickSpinBoxPrivate::updateValue()
{
    Q_Q(QQuickSpinBox);
    if (contentItem) {
        QVariant text = contentItem->property("text");
        if (text.isValid()) {
            int val = 0;
            QQmlEngine *engine = qmlEngine(q);
            if (engine && valueFromText.isCallable()) {
                QV4::ExecutionEngine *v4 = QQmlEnginePrivate::getV4Engine(engine);
                QJSValue loc(v4, QQmlLocale::wrap(v4, locale));
                val = valueFromText.call(QJSValueList() << text.toString() << loc).toInt();
            } else {
                val = locale.toInt(text.toString());
            }
            setValue(val, /* allowWrap = */ false, /* modified = */ true);
        }
    }
}

bool QQuickSpinBoxPrivate::setValue(int newValue, bool allowWrap, bool modified)
{
    Q_Q(QQuickSpinBox);
    if (q->isComponentComplete())
        newValue = boundValue(newValue, allowWrap);

    if (value == newValue)
        return false;

    value = newValue;

    updateDisplayText();
    updateUpEnabled();
    updateDownEnabled();

    emit q->valueChanged();
    if (modified)
        emit q->valueModified();
    return true;
}

bool QQuickSpinBoxPrivate::stepBy(int steps, bool modified)
{
    return setValue(value + steps, wrap, modified);
}

void QQuickSpinBoxPrivate::increase(bool modified)
{
    setValue(value + effectiveStepSize(), wrap, modified);
}

void QQuickSpinBoxPrivate::decrease(bool modified)
{
    setValue(value - effectiveStepSize(), wrap, modified);
}

int QQuickSpinBoxPrivate::effectiveStepSize() const
{
    return from > to ? -1 * stepSize : stepSize;
}

void QQuickSpinBoxPrivate::updateDisplayText()
{
    Q_Q(QQuickSpinBox);
    QString text;
    QQmlEngine *engine = qmlEngine(q);
    if (engine && textFromValue.isCallable()) {
        QV4::ExecutionEngine *v4 = QQmlEnginePrivate::getV4Engine(engine);
        QJSValue loc(v4, QQmlLocale::wrap(v4, locale));
        text = textFromValue.call(QJSValueList() << value << loc).toString();
    } else {
        text = locale.toString(value);
    }
    setDisplayText(text);
}

void QQuickSpinBoxPrivate::setDisplayText(const QString &text)
{
    Q_Q(QQuickSpinBox);
    if (displayText == text)
        return;

    displayText = text;
    emit q->displayTextChanged();
}

bool QQuickSpinBoxPrivate::upEnabled() const
{
    const QQuickItem *upIndicator = up->indicator();
    return upIndicator && upIndicator->isEnabled();
}

void QQuickSpinBoxPrivate::updateUpEnabled()
{
    QQuickItem *upIndicator = up->indicator();
    if (!upIndicator)
        return;

    upIndicator->setEnabled(wrap || (from < to ? value < to : value > to));
}

bool QQuickSpinBoxPrivate::downEnabled() const
{
    const QQuickItem *downIndicator = down->indicator();
    return downIndicator && downIndicator->isEnabled();
}

void QQuickSpinBoxPrivate::updateDownEnabled()
{
    QQuickItem *downIndicator = down->indicator();
    if (!downIndicator)
        return;

    downIndicator->setEnabled(wrap || (from < to ? value > from : value < from));
}

void QQuickSpinBoxPrivate::updateHover(const QPointF &pos)
{
    Q_Q(QQuickSpinBox);
    QQuickItem *ui = up->indicator();
    QQuickItem *di = down->indicator();
    up->setHovered(ui && ui->isEnabled() && ui->contains(q->mapToItem(ui, pos)));
    down->setHovered(di && di->isEnabled() && di->contains(q->mapToItem(di, pos)));
}

void QQuickSpinBoxPrivate::startRepeatDelay()
{
    Q_Q(QQuickSpinBox);
    stopPressRepeat();
    delayTimer = q->startTimer(AUTO_REPEAT_DELAY);
}

void QQuickSpinBoxPrivate::startPressRepeat()
{
    Q_Q(QQuickSpinBox);
    stopPressRepeat();
    repeatTimer = q->startTimer(AUTO_REPEAT_INTERVAL);
}

void QQuickSpinBoxPrivate::stopPressRepeat()
{
    Q_Q(QQuickSpinBox);
    if (delayTimer > 0) {
        q->killTimer(delayTimer);
        delayTimer = 0;
    }
    if (repeatTimer > 0) {
        q->killTimer(repeatTimer);
        repeatTimer = 0;
    }
}

void QQuickSpinBoxPrivate::handlePress(const QPointF &point)
{
    Q_Q(QQuickSpinBox);
    QQuickControlPrivate::handlePress(point);
    QQuickItem *ui = up->indicator();
    QQuickItem *di = down->indicator();
    up->setPressed(ui && ui->isEnabled() && ui->contains(ui->mapFromItem(q, point)));
    down->setPressed(di && di->isEnabled() && di->contains(di->mapFromItem(q, point)));

    bool pressed = up->isPressed() || down->isPressed();
    q->setAccessibleProperty("pressed", pressed);
    if (pressed)
        startRepeatDelay();
}

void QQuickSpinBoxPrivate::handleMove(const QPointF &point)
{
    Q_Q(QQuickSpinBox);
    QQuickControlPrivate::handleMove(point);
    QQuickItem *ui = up->indicator();
    QQuickItem *di = down->indicator();
    up->setHovered(ui && ui->isEnabled() && ui->contains(ui->mapFromItem(q, point)));
    up->setPressed(up->isHovered());
    down->setHovered(di && di->isEnabled() && di->contains(di->mapFromItem(q, point)));
    down->setPressed(down->isHovered());

    bool pressed = up->isPressed() || down->isPressed();
    q->setAccessibleProperty("pressed", pressed);
    if (!pressed)
        stopPressRepeat();
}

void QQuickSpinBoxPrivate::handleRelease(const QPointF &point)
{
    Q_Q(QQuickSpinBox);
    QQuickControlPrivate::handleRelease(point);
    QQuickItem *ui = up->indicator();
    QQuickItem *di = down->indicator();

    int oldValue = value;
    if (up->isPressed()) {
        up->setPressed(false);
        if (repeatTimer <= 0 && ui && ui->contains(ui->mapFromItem(q, point)))
            q->increase();
    } else if (down->isPressed()) {
        down->setPressed(false);
        if (repeatTimer <= 0 && di && di->contains(di->mapFromItem(q, point)))
            q->decrease();
    }
    if (value != oldValue)
        emit q->valueModified();

    q->setAccessibleProperty("pressed", false);
    stopPressRepeat();
}

void QQuickSpinBoxPrivate::handleUngrab()
{
    Q_Q(QQuickSpinBox);
    QQuickControlPrivate::handleUngrab();
    up->setPressed(false);
    down->setPressed(false);

    q->setAccessibleProperty("pressed", false);
    stopPressRepeat();
}

void QQuickSpinBoxPrivate::itemImplicitWidthChanged(QQuickItem *item)
{
    QQuickControlPrivate::itemImplicitWidthChanged(item);
    if (item == up->indicator())
        emit up->implicitIndicatorWidthChanged();
    else if (item == down->indicator())
        emit down->implicitIndicatorWidthChanged();
}

void QQuickSpinBoxPrivate::itemImplicitHeightChanged(QQuickItem *item)
{
    QQuickControlPrivate::itemImplicitHeightChanged(item);
    if (item == up->indicator())
        emit up->implicitIndicatorHeightChanged();
    else if (item == down->indicator())
        emit down->implicitIndicatorHeightChanged();
}

QQuickSpinBox::QQuickSpinBox(QQuickItem *parent)
    : QQuickControl(*(new QQuickSpinBoxPrivate), parent)
{
    Q_D(QQuickSpinBox);
    d->up = new QQuickSpinButton(this);
    d->down = new QQuickSpinButton(this);

    setFlag(ItemIsFocusScope);
    setFiltersChildMouseEvents(true);
    setAcceptedMouseButtons(Qt::LeftButton);
#if QT_CONFIG(cursor)
    setCursor(Qt::ArrowCursor);
#endif
}

QQuickSpinBox::~QQuickSpinBox()
{
    Q_D(QQuickSpinBox);
    d->removeImplicitSizeListener(d->up->indicator());
    d->removeImplicitSizeListener(d->down->indicator());
}

/*!
    \qmlproperty int QtQuick.Controls::SpinBox::from

    This property holds the starting value for the range. The default value is \c 0.

    \sa to, value
*/
int QQuickSpinBox::from() const
{
    Q_D(const QQuickSpinBox);
    return d->from;
}

void QQuickSpinBox::setFrom(int from)
{
    Q_D(QQuickSpinBox);
    if (d->from == from)
        return;

    d->from = from;
    emit fromChanged();
    if (isComponentComplete()) {
        if (!d->setValue(d->value, /* allowWrap = */ false, /* modified = */ false)) {
            d->updateUpEnabled();
            d->updateDownEnabled();
        }
    }
}

/*!
    \qmlproperty int QtQuick.Controls::SpinBox::to

    This property holds the end value for the range. The default value is \c 99.

    \sa from, value
*/
int QQuickSpinBox::to() const
{
    Q_D(const QQuickSpinBox);
    return d->to;
}

void QQuickSpinBox::setTo(int to)
{
    Q_D(QQuickSpinBox);
    if (d->to == to)
        return;

    d->to = to;
    emit toChanged();
    if (isComponentComplete()) {
        if (!d->setValue(d->value, /* allowWrap = */false, /* modified = */ false)) {
            d->updateUpEnabled();
            d->updateDownEnabled();
        }
    }
}

/*!
    \qmlproperty int QtQuick.Controls::SpinBox::value

    This property holds the value in the range \c from - \c to. The default value is \c 0.
*/
int QQuickSpinBox::value() const
{
    Q_D(const QQuickSpinBox);
    return d->value;
}

void QQuickSpinBox::setValue(int value)
{
    Q_D(QQuickSpinBox);
    d->setValue(value, /* allowWrap = */ false, /* modified = */ false);
}

/*!
    \qmlproperty int QtQuick.Controls::SpinBox::stepSize

    This property holds the step size. The default value is \c 1.

    \sa increase(), decrease()
*/
int QQuickSpinBox::stepSize() const
{
    Q_D(const QQuickSpinBox);
    return d->stepSize;
}

void QQuickSpinBox::setStepSize(int step)
{
    Q_D(QQuickSpinBox);
    if (d->stepSize == step)
        return;

    d->stepSize = step;
    emit stepSizeChanged();
}

/*!
    \qmlproperty bool QtQuick.Controls::SpinBox::editable

    This property holds whether the spinbox is editable. The default value is \c false.

    \sa validator
*/
bool QQuickSpinBox::isEditable() const
{
    Q_D(const QQuickSpinBox);
    return d->editable;
}

void QQuickSpinBox::setEditable(bool editable)
{
    Q_D(QQuickSpinBox);
    if (d->editable == editable)
        return;

#if QT_CONFIG(cursor)
    if (d->contentItem) {
        if (editable)
            d->contentItem->setCursor(Qt::IBeamCursor);
        else
            d->contentItem->unsetCursor();
    }
#endif

    d->editable = editable;
    setAccessibleProperty("editable", editable);
    emit editableChanged();
}

/*!
    \qmlproperty Validator QtQuick.Controls::SpinBox::validator

    This property holds the input text validator for editable spinboxes. By
    default, SpinBox uses \l IntValidator to accept input of integer numbers.

    \code
    SpinBox {
        id: control
        validator: IntValidator {
            locale: control.locale.name
            bottom: Math.min(control.from, control.to)
            top: Math.max(control.from, control.to)
        }
    }
    \endcode

    \sa editable, textFromValue, valueFromText, {Control::locale}{locale}
*/
QValidator *QQuickSpinBox::validator() const
{
    Q_D(const QQuickSpinBox);
    return d->validator;
}

void QQuickSpinBox::setValidator(QValidator *validator)
{
    Q_D(QQuickSpinBox);
    if (d->validator == validator)
        return;

    d->validator = validator;
    emit validatorChanged();
}

/*!
    \qmlproperty function QtQuick.Controls::SpinBox::textFromValue

    This property holds a callback function that is called whenever
    an integer value needs to be converted to display text.

    The default function can be overridden to display custom text for a given
    value. This applies to both editable and non-editable spinboxes;
    for example, when using the up and down buttons or a mouse wheel to
    increment and decrement the value, the new value is converted to display
    text using this function.

    The callback function signature is \c {string function(value, locale)}.
    The function can have one or two arguments, where the first argument
    is the value to be converted, and the optional second argument is the
    locale that should be used for the conversion, if applicable.

    The default implementation does the conversion using
    \l {QtQml::Number::toLocaleString()}{Number.toLocaleString}():

    \code
    textFromValue: function(value, locale) { return Number(value).toLocaleString(locale, 'f', 0); }
    \endcode

    \note When applying a custom \c textFromValue implementation for editable
    spinboxes, a matching \l valueFromText implementation must be provided
    to be able to convert the custom text back to an integer value.

    \sa valueFromText, validator, {Control::locale}{locale}
*/
QJSValue QQuickSpinBox::textFromValue() const
{
    Q_D(const QQuickSpinBox);
    if (!d->textFromValue.isCallable()) {
        QQmlEngine *engine = qmlEngine(this);
        if (engine)
            d->textFromValue = engine->evaluate(QStringLiteral("(function(value, locale) { return Number(value).toLocaleString(locale, 'f', 0); })"));
    }
    return d->textFromValue;
}

void QQuickSpinBox::setTextFromValue(const QJSValue &callback)
{
    Q_D(QQuickSpinBox);
    if (!callback.isCallable()) {
        qmlWarning(this) << "textFromValue must be a callable function";
        return;
    }
    d->textFromValue = callback;
    emit textFromValueChanged();
}

/*!
    \qmlproperty function QtQuick.Controls::SpinBox::valueFromText

    This property holds a callback function that is called whenever
    input text needs to be converted to an integer value.

    This function only needs to be overridden when \l textFromValue
    is overridden for an editable spinbox.

    The callback function signature is \c {int function(text, locale)}.
    The function can have one or two arguments, where the first argument
    is the text to be converted, and the optional second argument is the
    locale that should be used for the conversion, if applicable.

    The default implementation does the conversion using \l {QtQml::Locale}{Number.fromLocaleString()}:

    \code
    valueFromText: function(text, locale) { return Number.fromLocaleString(locale, text); }
    \endcode

    \note When applying a custom \l textFromValue implementation for editable
    spinboxes, a matching \c valueFromText implementation must be provided
    to be able to convert the custom text back to an integer value.

    \sa textFromValue, validator, {Control::locale}{locale}
*/
QJSValue QQuickSpinBox::valueFromText() const
{
    Q_D(const QQuickSpinBox);
    if (!d->valueFromText.isCallable()) {
        QQmlEngine *engine = qmlEngine(this);
        if (engine)
            d->valueFromText = engine->evaluate(QStringLiteral("function(text, locale) { return Number.fromLocaleString(locale, text); }"));
    }
    return d->valueFromText;
}

void QQuickSpinBox::setValueFromText(const QJSValue &callback)
{
    Q_D(QQuickSpinBox);
    if (!callback.isCallable()) {
        qmlWarning(this) << "valueFromText must be a callable function";
        return;
    }
    d->valueFromText = callback;
    emit valueFromTextChanged();
}

/*!
    \qmlpropertygroup QtQuick.Controls::SpinBox::up
    \qmlproperty bool QtQuick.Controls::SpinBox::up.pressed
    \qmlproperty Item QtQuick.Controls::SpinBox::up.indicator
    \qmlproperty bool QtQuick.Controls::SpinBox::up.hovered
    \qmlproperty real QtQuick.Controls::SpinBox::up.implicitIndicatorWidth
    \qmlproperty real QtQuick.Controls::SpinBox::up.implicitIndicatorHeight

    These properties hold the up indicator item and whether it is pressed or
    hovered. The \c up.hovered property was introduced in QtQuick.Controls 2.1,
    and the \c up.implicitIndicatorWidth and \c up.implicitIndicatorHeight
    properties were introduced in QtQuick.Controls 2.5.

    \sa increase()
*/
QQuickSpinButton *QQuickSpinBox::up() const
{
    Q_D(const QQuickSpinBox);
    return d->up;
}

/*!
    \qmlpropertygroup QtQuick.Controls::SpinBox::down
    \qmlproperty bool QtQuick.Controls::SpinBox::down.pressed
    \qmlproperty Item QtQuick.Controls::SpinBox::down.indicator
    \qmlproperty bool QtQuick.Controls::SpinBox::down.hovered
    \qmlproperty real QtQuick.Controls::SpinBox::down.implicitIndicatorWidth
    \qmlproperty real QtQuick.Controls::SpinBox::down.implicitIndicatorHeight

    These properties hold the down indicator item and whether it is pressed or
    hovered. The \c down.hovered property was introduced in QtQuick.Controls 2.1,
    and the \c down.implicitIndicatorWidth and \c down.implicitIndicatorHeight
    properties were introduced in QtQuick.Controls 2.5.

    \sa decrease()
*/
QQuickSpinButton *QQuickSpinBox::down() const
{
    Q_D(const QQuickSpinBox);
    return d->down;
}

/*!
    \since QtQuick.Controls 2.2 (Qt 5.9)
    \qmlproperty flags QtQuick.Controls::SpinBox::inputMethodHints

    This property provides hints to the input method about the expected content
    of the spin box and how it should operate.

    The default value is \c Qt.ImhDigitsOnly.

    \include inputmethodhints.qdocinc
*/
Qt::InputMethodHints QQuickSpinBox::inputMethodHints() const
{
    Q_D(const QQuickSpinBox);
    return d->inputMethodHints;
}

void QQuickSpinBox::setInputMethodHints(Qt::InputMethodHints hints)
{
    Q_D(QQuickSpinBox);
    if (d->inputMethodHints == hints)
        return;

    d->inputMethodHints = hints;
    emit inputMethodHintsChanged();
}

/*!
    \since QtQuick.Controls 2.2 (Qt 5.9)
    \qmlproperty bool QtQuick.Controls::SpinBox::inputMethodComposing
    \readonly

    This property holds whether an editable spin box has partial text input from an input method.

    While it is composing, an input method may rely on mouse or key events from the spin box to
    edit or commit the partial text. This property can be used to determine when to disable event
    handlers that may interfere with the correct operation of an input method.
*/
bool QQuickSpinBox::isInputMethodComposing() const
{
    Q_D(const QQuickSpinBox);
    return d->contentItem && d->contentItem->property("inputMethodComposing").toBool();
}

/*!
    \since QtQuick.Controls 2.3 (Qt 5.10)
    \qmlproperty bool QtQuick.Controls::SpinBox::wrap

    This property holds whether the spinbox wraps. The default value is \c false.

    If wrap is \c true, stepping past \l to changes the value to \l from and vice versa.
*/
bool QQuickSpinBox::wrap() const
{
    Q_D(const QQuickSpinBox);
    return d->wrap;
}

void QQuickSpinBox::setWrap(bool wrap)
{
    Q_D(QQuickSpinBox);
    if (d->wrap == wrap)
        return;

    d->wrap = wrap;
    if (d->value == d->from || d->value == d->to) {
        d->updateUpEnabled();
        d->updateDownEnabled();
    }
    emit wrapChanged();
}

/*!
    \since QtQuick.Controls 2.4 (Qt 5.11)
    \qmlproperty string QtQuick.Controls::SpinBox::displayText
    \readonly

    This property holds the textual value of the spinbox.

    The value of the property is based on \l textFromValue and \l {Control::}
    {locale}, and equal to:
    \badcode
    var text = spinBox.textFromValue(spinBox.value, spinBox.locale)
    \endcode

    \sa textFromValue
*/
QString QQuickSpinBox::displayText() const
{
    Q_D(const QQuickSpinBox);
    return d->displayText;
}

/*!
    \qmlmethod void QtQuick.Controls::SpinBox::increase()

    Increases the value by \l stepSize, or \c 1 if stepSize is not defined.

    \sa stepSize
*/
void QQuickSpinBox::increase()
{
    Q_D(QQuickSpinBox);
    d->increase(false);
}

/*!
    \qmlmethod void QtQuick.Controls::SpinBox::decrease()

    Decreases the value by \l stepSize, or \c 1 if stepSize is not defined.

    \sa stepSize
*/
void QQuickSpinBox::decrease()
{
    Q_D(QQuickSpinBox);
    d->decrease(false);
}

void QQuickSpinBox::focusInEvent(QFocusEvent *event)
{
    Q_D(QQuickSpinBox);
    QQuickControl::focusInEvent(event);

    // When an editable SpinBox gets focus, it must pass on the focus to its editor.
    if (d->editable && d->contentItem && !d->contentItem->hasActiveFocus())
        d->contentItem->forceActiveFocus(event->reason());
}

void QQuickSpinBox::hoverEnterEvent(QHoverEvent *event)
{
    Q_D(QQuickSpinBox);
    QQuickControl::hoverEnterEvent(event);
    d->updateHover(event->posF());
}

void QQuickSpinBox::hoverMoveEvent(QHoverEvent *event)
{
    Q_D(QQuickSpinBox);
    QQuickControl::hoverMoveEvent(event);
    d->updateHover(event->posF());
}

void QQuickSpinBox::hoverLeaveEvent(QHoverEvent *event)
{
    Q_D(QQuickSpinBox);
    QQuickControl::hoverLeaveEvent(event);
    d->down->setHovered(false);
    d->up->setHovered(false);
}

void QQuickSpinBox::keyPressEvent(QKeyEvent *event)
{
    Q_D(QQuickSpinBox);
    QQuickControl::keyPressEvent(event);

    switch (event->key()) {
    case Qt::Key_Up:
        if (d->upEnabled()) {
            d->increase(true);
            d->up->setPressed(true);
            event->accept();
        }
        break;

    case Qt::Key_Down:
        if (d->downEnabled()) {
            d->decrease(true);
            d->down->setPressed(true);
            event->accept();
        }
        break;

    default:
        break;
    }

    setAccessibleProperty("pressed", d->up->isPressed() || d->down->isPressed());
}

void QQuickSpinBox::keyReleaseEvent(QKeyEvent *event)
{
    Q_D(QQuickSpinBox);
    QQuickControl::keyReleaseEvent(event);

    if (d->editable && (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return))
        d->updateValue();

    d->up->setPressed(false);
    d->down->setPressed(false);
    setAccessibleProperty("pressed", false);
}

void QQuickSpinBox::timerEvent(QTimerEvent *event)
{
    Q_D(QQuickSpinBox);
    QQuickControl::timerEvent(event);
    if (event->timerId() == d->delayTimer) {
        d->startPressRepeat();
    } else if (event->timerId() == d->repeatTimer) {
        if (d->up->isPressed())
            d->increase(true);
        else if (d->down->isPressed())
            d->decrease(true);
    }
}

#if QT_CONFIG(wheelevent)
void QQuickSpinBox::wheelEvent(QWheelEvent *event)
{
    Q_D(QQuickSpinBox);
    QQuickControl::wheelEvent(event);
    if (d->wheelEnabled) {
        const QPointF angle = event->angleDelta();
        const qreal delta = (qFuzzyIsNull(angle.y()) ? angle.x() : angle.y()) / QWheelEvent::DefaultDeltasPerStep;
        d->stepBy(qRound(d->effectiveStepSize() * delta), true);
    }
}
#endif

void QQuickSpinBox::classBegin()
{
    Q_D(QQuickSpinBox);
    QQuickControl::classBegin();

    QQmlContext *context = qmlContext(this);
    if (context) {
        QQmlEngine::setContextForObject(d->up, context);
        QQmlEngine::setContextForObject(d->down, context);
    }
}

void QQuickSpinBox::componentComplete()
{
    Q_D(QQuickSpinBox);
    QQuickSpinButtonPrivate::get(d->up)->executeIndicator(true);
    QQuickSpinButtonPrivate::get(d->down)->executeIndicator(true);

    QQuickControl::componentComplete();
    if (!d->setValue(d->value, /* allowWrap = */ false, /* modified = */ false)) {
        d->updateDisplayText();
        d->updateUpEnabled();
        d->updateDownEnabled();
    }
}

void QQuickSpinBox::itemChange(ItemChange change, const ItemChangeData &value)
{
    Q_D(QQuickSpinBox);
    QQuickControl::itemChange(change, value);
    if (d->editable && change == ItemActiveFocusHasChanged && !value.boolValue)
        d->updateValue();
}

void QQuickSpinBox::contentItemChange(QQuickItem *newItem, QQuickItem *oldItem)
{
    Q_D(QQuickSpinBox);
    if (QQuickTextInput *oldInput = qobject_cast<QQuickTextInput *>(oldItem))
        disconnect(oldInput, &QQuickTextInput::inputMethodComposingChanged, this, &QQuickSpinBox::inputMethodComposingChanged);

    if (newItem) {
        newItem->setActiveFocusOnTab(true);
        if (d->activeFocus)
            newItem->forceActiveFocus(d->focusReason);
#if QT_CONFIG(cursor)
        if (d->editable)
            newItem->setCursor(Qt::IBeamCursor);
#endif

        if (QQuickTextInput *newInput = qobject_cast<QQuickTextInput *>(newItem))
            connect(newInput, &QQuickTextInput::inputMethodComposingChanged, this, &QQuickSpinBox::inputMethodComposingChanged);
    }
}

void QQuickSpinBox::localeChange(const QLocale &newLocale, const QLocale &oldLocale)
{
    Q_D(QQuickSpinBox);
    QQuickControl::localeChange(newLocale, oldLocale);
    d->updateDisplayText();
}

QFont QQuickSpinBox::defaultFont() const
{
    return QQuickTheme::font(QQuickTheme::SpinBox);
}

QPalette QQuickSpinBox::defaultPalette() const
{
    return QQuickTheme::palette(QQuickTheme::SpinBox);
}

#if QT_CONFIG(accessibility)
QAccessible::Role QQuickSpinBox::accessibleRole() const
{
    return QAccessible::SpinBox;
}

void QQuickSpinBox::accessibilityActiveChanged(bool active)
{
    Q_D(QQuickSpinBox);
    QQuickControl::accessibilityActiveChanged(active);

    if (active)
        setAccessibleProperty("editable", d->editable);
}
#endif

static inline QString indicatorName() { return QStringLiteral("indicator"); }

void QQuickSpinButtonPrivate::cancelIndicator()
{
    Q_Q(QQuickSpinButton);
    quickCancelDeferred(q, indicatorName());
}

void QQuickSpinButtonPrivate::executeIndicator(bool complete)
{
    Q_Q(QQuickSpinButton);
    if (indicator.wasExecuted())
        return;

    if (!indicator || complete)
        quickBeginDeferred(q, indicatorName(), indicator);
    if (complete)
        quickCompleteDeferred(q, indicatorName(), indicator);
}

QQuickSpinButton::QQuickSpinButton(QQuickSpinBox *parent)
    : QObject(*(new QQuickSpinButtonPrivate), parent)
{
}

bool QQuickSpinButton::isPressed() const
{
    Q_D(const QQuickSpinButton);
    return d->pressed;
}

void QQuickSpinButton::setPressed(bool pressed)
{
    Q_D(QQuickSpinButton);
    if (d->pressed == pressed)
        return;

    d->pressed = pressed;
    emit pressedChanged();
}

QQuickItem *QQuickSpinButton::indicator() const
{
    QQuickSpinButtonPrivate *d = const_cast<QQuickSpinButtonPrivate *>(d_func());
    if (!d->indicator)
        d->executeIndicator();
    return d->indicator;
}

void QQuickSpinButton::setIndicator(QQuickItem *indicator)
{
    Q_D(QQuickSpinButton);
    if (d->indicator == indicator)
        return;

    if (!d->indicator.isExecuting())
        d->cancelIndicator();

    const qreal oldImplicitIndicatorWidth = implicitIndicatorWidth();
    const qreal oldImplicitIndicatorHeight = implicitIndicatorHeight();

    QQuickSpinBox *spinBox = static_cast<QQuickSpinBox *>(parent());
    QQuickSpinBoxPrivate::get(spinBox)->removeImplicitSizeListener(d->indicator);
    delete d->indicator;
    d->indicator = indicator;

    if (indicator) {
        if (!indicator->parentItem())
            indicator->setParentItem(spinBox);
        QQuickSpinBoxPrivate::get(spinBox)->addImplicitSizeListener(indicator);
    }

    if (!qFuzzyCompare(oldImplicitIndicatorWidth, implicitIndicatorWidth()))
        emit implicitIndicatorWidthChanged();
    if (!qFuzzyCompare(oldImplicitIndicatorHeight, implicitIndicatorHeight()))
        emit implicitIndicatorHeightChanged();
    if (!d->indicator.isExecuting())
        emit indicatorChanged();
}

bool QQuickSpinButton::isHovered() const
{
    Q_D(const QQuickSpinButton);
    return d->hovered;
}

void QQuickSpinButton::setHovered(bool hovered)
{
    Q_D(QQuickSpinButton);
    if (d->hovered == hovered)
        return;

    d->hovered = hovered;
    emit hoveredChanged();
}

qreal QQuickSpinButton::implicitIndicatorWidth() const
{
    Q_D(const QQuickSpinButton);
    if (!d->indicator)
        return 0;
    return d->indicator->implicitWidth();
}

qreal QQuickSpinButton::implicitIndicatorHeight() const
{
    Q_D(const QQuickSpinButton);
    if (!d->indicator)
        return 0;
    return d->indicator->implicitHeight();
}

QT_END_NAMESPACE
