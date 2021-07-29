/****************************************************************************
**
** Copyright (C) 2018 Governikus GmbH & Co. KG
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtNfc module of the Qt Toolkit.
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

package org.qtproject.qt5.android.nfc;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.nfc.NfcAdapter;


public class QtNfcBroadcastReceiver extends BroadcastReceiver
{
    private Context qtContext;

    public QtNfcBroadcastReceiver(Context context)
    {
        qtContext = context;
        IntentFilter filter = new IntentFilter(NfcAdapter.ACTION_ADAPTER_STATE_CHANGED);
        qtContext.registerReceiver(this, filter);
    }

    public void unregisterReceiver()
    {
        qtContext.unregisterReceiver(this);
    }

    public void onReceive(Context context, Intent intent)
    {
        final int state = intent.getIntExtra(NfcAdapter.EXTRA_ADAPTER_STATE, NfcAdapter.STATE_OFF);
        jniOnReceive(state);
    }

    public native void jniOnReceive(int state);
}
