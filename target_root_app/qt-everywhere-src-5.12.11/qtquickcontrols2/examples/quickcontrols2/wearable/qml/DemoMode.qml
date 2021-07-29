/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
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

import QtQuick 2.12
import QtQuick.Controls 2.12

import "Style"

Item {
    property StackView stackView

    SequentialAnimation {
        id: demoModeAnimation
        running: settings.demoMode

        // Set brightness back to normal.
        ScriptAction { script: settings.brightness = 0 }

        // Go back to the launcher page.
        PauseAnimation { duration: 1000 }
        ScriptAction { script: stackView.pop(null) }
        PauseAnimation { duration: 2000 }

        // Open the world clock page.
        ScriptAction { script: stackView.currentItem.launched(Qt.resolvedUrl("WorldClock/WorldClockPage.qml")) }
        PauseAnimation { duration: 2000 }

        // Swipe across a few times.
        SequentialAnimation {
            loops: 6

            ScriptAction { script: stackView.currentItem.children[0].incrementCurrentIndex() }
            PauseAnimation { duration: 2500 }
        }


        // Go back to the launcher page.
        ScriptAction { script: stackView.pop(null) }
        PauseAnimation { duration: 2000 }

        // Open the navigation page.
        ScriptAction { script: stackView.currentItem.incrementCurrentIndex() }
        PauseAnimation { duration: 1000 }
        ScriptAction { script: stackView.currentItem.launched(Qt.resolvedUrl("Navigation/NavigationPage.qml")) }
        PauseAnimation { duration: 2000 }

        // Flick down a few times.
        SequentialAnimation {
            loops: 6

            ScriptAction { script: stackView.currentItem.routeListView.incrementCurrentIndex() }
            PauseAnimation { duration: 2000 }
        }


        // Go back to the launcher page.
        ScriptAction { script: stackView.pop(null) }
        PauseAnimation { duration: 2000 }

        // Open the weather page.
        ScriptAction { script: stackView.currentItem.incrementCurrentIndex() }
        PauseAnimation { duration: 1000 }
        ScriptAction { script: stackView.currentItem.launched(Qt.resolvedUrl("Weather/WeatherPage.qml")) }
        PauseAnimation { duration: 2000 }

        // Swipe across a few times.
        SequentialAnimation {
            loops: 4

            ScriptAction { script: stackView.currentItem.children[0].incrementCurrentIndex() }
            PauseAnimation { duration: 2000 }
        }


        // Go back to the launcher page.
        ScriptAction { script: stackView.pop(null) }
        PauseAnimation { duration: 2000 }

        // Open the fitness page.
        ScriptAction { script: stackView.currentItem.incrementCurrentIndex() }
        PauseAnimation { duration: 1000 }
        ScriptAction { script: stackView.currentItem.launched(Qt.resolvedUrl("Fitness/FitnessPage.qml")) }
        PauseAnimation { duration: 2000 }

        // Swipe across a few times.
        SequentialAnimation {
            loops: 2

            ScriptAction { script: stackView.currentItem.children[0].incrementCurrentIndex() }
            PauseAnimation { duration: 2000 }
        }


        // Go back to the launcher page.
        ScriptAction { script: stackView.pop(null) }
        PauseAnimation { duration: 2000 }

        // Open the notifications page.
        ScriptAction { script: stackView.currentItem.incrementCurrentIndex() }
        PauseAnimation { duration: 1000 }
        ScriptAction { script: stackView.currentItem.launched(Qt.resolvedUrl("Notifications/NotificationsPage.qml")) }

        // Flick down a few times.
        SequentialAnimation {
            loops: 3

            PauseAnimation { duration: 2000 }
            ScriptAction { script: stackView.currentItem.incrementCurrentIndex() }
        }


        // Go back to the launcher page.
        ScriptAction { script: stackView.pop(null) }
        PauseAnimation { duration: 2000 }

        // Open the alarms page.
        ScriptAction { script: stackView.currentItem.incrementCurrentIndex() }
        PauseAnimation { duration: 1000 }
        ScriptAction { script: stackView.currentItem.launched(Qt.resolvedUrl("Alarms/AlarmsPage.qml")) }
        PauseAnimation { duration: 2000 }

        // Toggle the switch.
        ScriptAction { script: stackView.currentItem.children[0].currentItem.stateSwitch.toggle() }
        PauseAnimation { duration: 2000 }

        // Go to the next alarm.
        ScriptAction { script: stackView.currentItem.children[0].incrementCurrentIndex() }
        PauseAnimation { duration: 2000 }

        // Toggle the switch there too.
        ScriptAction { script: stackView.currentItem.children[0].currentItem.stateSwitch.toggle() }
        PauseAnimation { duration: 2000 }


        // Go back to the launcher page.
        ScriptAction { script: stackView.pop(null) }
        PauseAnimation { duration: 2000 }

        // Open the settings page.
        ScriptAction { script: stackView.currentItem.incrementCurrentIndex() }
        PauseAnimation { duration: 1000 }
        ScriptAction { script: stackView.currentItem.launched(Qt.resolvedUrl("Settings/SettingsPage.qml")) }
        PauseAnimation { duration: 3000 }

        // Toggle the switches.
        ScriptAction { script: stackView.currentItem.children[0].currentItem.bluetoothSwitch.toggle() }
        PauseAnimation { duration: 1000 }
        ScriptAction { script: stackView.currentItem.children[0].currentItem.wirelessSwitch.toggle() }
        PauseAnimation { duration: 3000 }

        // Go to the next page.
        ScriptAction { script: stackView.currentItem.children[0].incrementCurrentIndex() }

        // Play with the brightness slider.
        // First, set it to full brightness so we start in the correct state.
        ScriptAction {
            script: {
                var brightnessSlider = stackView.currentItem.children[0].currentItem.brightnessSlider
                brightnessSlider.value = 0
                // increase()/decrease() are not a result of user interaction and
                // hence moved() will not be emitted, so we do it ourselves.
                brightnessSlider.moved()
            }
        }

        // Decrease the brightness.
        SequentialAnimation {
            loops: 3

            PauseAnimation { duration: 1000 }
            ScriptAction {
                script: {
                    var brightnessSlider = stackView.currentItem.children[0].currentItem.brightnessSlider
                    brightnessSlider.decrease()
                    brightnessSlider.moved()
                }
            }
        }

        // Increase the brightness back to full.
        PauseAnimation { duration: 3000 }
        SequentialAnimation {
            loops: 3

            PauseAnimation { duration: 1000 }
            ScriptAction {
                script: {
                    var brightnessSlider = stackView.currentItem.children[0].currentItem.brightnessSlider
                    brightnessSlider.increase()
                    brightnessSlider.moved()
                }
            }
        }

        // Toggle the dark theme switch.
        PauseAnimation { duration: 2000 }
        ScriptAction {
            script: {
                var darkThemeSwitch = stackView.currentItem.children[0].currentItem.darkThemeSwitch
                darkThemeSwitch.toggle()
                // As above, only proper user interaction results in toggled() being emitted,
                // so we do it ourselves.
                darkThemeSwitch.toggled()
            }
        }
        PauseAnimation { duration: 4000 }

        // Go back to the launcher page.
        ScriptAction { script: stackView.pop(null) }
    }
}
