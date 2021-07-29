/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
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

#include "browsing_data_remover_delegate_qt.h"

#include "base/bind.h"
#include "base/callback.h"
#include "components/web_cache/browser/web_cache_manager.h"
#include "content/public/browser/browsing_data_remover.h"

#include <QtGlobal>

namespace QtWebEngineCore {

bool DoesOriginMatchEmbedderMask(int origin_type_mask,
                                 const GURL &origin,
                                 storage::SpecialStoragePolicy *policy) {
    Q_UNUSED(origin_type_mask);
    Q_UNUSED(origin);
    Q_UNUSED(policy);
    return true;
}

content::BrowsingDataRemoverDelegate::EmbedderOriginTypeMatcher BrowsingDataRemoverDelegateQt::GetOriginTypeMatcher() const {
    return base::Bind(&DoesOriginMatchEmbedderMask);
}

bool BrowsingDataRemoverDelegateQt::MayRemoveDownloadHistory() const {
    return true;
}

void BrowsingDataRemoverDelegateQt::RemoveEmbedderData(
            const base::Time &delete_begin,
            const base::Time &delete_end,
            int remove_mask,
            const content::BrowsingDataFilterBuilder& filter_builder,
            int origin_type_mask,
            base::OnceClosure callback) {
    Q_UNUSED(delete_begin);
    Q_UNUSED(delete_end);
    Q_UNUSED(filter_builder);
    Q_UNUSED(origin_type_mask);

    if (remove_mask & content::BrowsingDataRemover::DATA_TYPE_CACHE)
        web_cache::WebCacheManager::GetInstance()->ClearCache();

    std::move(callback).Run();
}

} // namespace QtWebEngineCore
