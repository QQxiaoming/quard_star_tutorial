/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the test suite of the Qt Toolkit.
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

#include "assetfixer.h"

#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QImage>
#include <QLoggingCategory>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QtMath>

#include "directoryvalidator.h"

Q_LOGGING_CATEGORY(lcAssetFixer, "qt.quick.controls.tools.testbench.assetfixer.brief")
Q_LOGGING_CATEGORY(lcAssetFixerVerbose, "qt.quick.controls.tools.testbench.assetfixer.verbose")

static const QColor black = Qt::black;
static const QColor red = Qt::red;

/*
    This class:

    - Watches a given asset directory for changes. When it notices a change in the directory's
      "last modification" time, it suggests that client code call fixAssets(). It suggests
      rather than just doing it itself because the client code (QML) may want to wait a second
      or two to see if more changes are coming before doing an expensive fixup, as exporting
      a bunch of files into a directory will cause several directoryChanged() emissions from
      QFileSystemWatcher.
    - Fixes 9-patch image assets via the function below.
*/

/*
    This function:

    - Crops the image to the area within the 9-patch lines if necessary.
      This can happen if e.g. a shadow is applied to an asset in Illustrator
      and it causes the image to be larger than necessary.
    - Reduces the thickness of the 9-patch lines. This is necessary to enable
      designers not to have to worry about creating one pixel-thick lines for
      each DPI variant of an asset; they can simply export the asset at each
      DPI variant as usual and this program will fix it for them.

      See README.md for more information.
*/
bool cropImageToLines(QImage *image)
{
    QRect cropArea;
    /*
        We need to keep track of this because of the following case:

           ______________________
           ______________________
        ||
          oooooooooooooooooooooooo
        ||

        If we didn't keep track of thickness, the top edge's lines would be found fine,
        but then we'd look at the bottom edge and we'd accidentally pick up the left edge's lines.
        Keeping track of thickness ensures that we have some way of knowing if we're far enough
        in for the line to belong to a certain edge.

        Note that this approach is still limited, as it doesn't account for the top edge,
        but we have to start somewhere in order to find the thickness.
    */
    int thickness = 0;

    bool cropTop = false;
    bool foundOnePixelThick9PatchLine = false;
    // We have to go row by row because otherwise we might find a pixel that
    // belongs to e.g. the left edge.
    for (int y = 0; y < qFloor(image->height() / 2.0) && !cropTop && !foundOnePixelThick9PatchLine; ++y) {
        for (int x = 1; x < image->width() - 2 && !cropTop && !foundOnePixelThick9PatchLine; ++x) {
            const QColor pixelColor = image->pixelColor(x, y);
            if (pixelColor == black || pixelColor == red) {
                if (y == 0) {
                    const QColor pixelColorBelow = image->pixelColor(x, y + 1);
                    if (pixelColorBelow != black && pixelColorBelow != red) {
                        // We've already found the top of the 9-patch line, and the row below it
                        // is a different color, so we know that it's one pixel thick, and that we're done.
                        // Note that we can't just assume all of the other edges are the same and return here,
                        // as we also need to account for e.g. shadows.
                        qCDebug(lcAssetFixerVerbose) << "found one-pixel-thick nine patch line on top edge at x" << x;
                        foundOnePixelThick9PatchLine = true;
                        thickness = 1;
                    }
                } else {
                    // It's not already at the top edge, so crop the top edge.
                    cropTop = true;

                    // Now that we've found the line, find out how thick it is.
                    for (int yy = y; yy < qFloor(image->height() / 2.0); ++yy) {
                        const QColor pixelColor = image->pixelColor(x, yy);
                        if (pixelColor == black || pixelColor == red) {
                            cropArea.setTop(yy);
                        } else {
                            break;
                        }
                    }

                    // + 1 for the pixel that we leave in when cropping,
                    // another +1 for the fact that this else statement is only entered when y > 0
                    if (thickness == 0) {
                        thickness = cropArea.top() - y + 2;
                        qCDebug(lcAssetFixerVerbose) << "found first croppable nine patch line on top edge at x" << x << "y" << y
                            << "with thickness" << thickness;
                    } else {
                        qCDebug(lcAssetFixerVerbose) << "found first croppable nine patch line on top edge at x" << x << "y" << y
                            << "using existing thickness of" << thickness;
                    }
                }
            }
        }
    }

    bool cropBottom = false;
    foundOnePixelThick9PatchLine = false;
    for (int y = image->height() - 1; y >= qCeil(image->height() / 2.0) && !cropBottom && !foundOnePixelThick9PatchLine; --y) {
        for (int x = qMax(1, thickness); x < image->width() - 2 && !cropBottom && !foundOnePixelThick9PatchLine; ++x) {
            const QColor pixelColor = image->pixelColor(x, y);
            if (pixelColor == black || pixelColor == red) {
                if (y == image->height() - 1) {
                    const QColor pixelColorAbove = image->pixelColor(x, y - 1);
                    if (pixelColorAbove != black && pixelColorAbove != red) {
                        // We've already found the bottom of the 9-patch line, and the row above it
                        // is a different color, so we know that it's one pixel thick, and that we're done.
                        qCDebug(lcAssetFixerVerbose) << "found one-pixel-thick nine patch line on bottom edge at x" << x;
                        foundOnePixelThick9PatchLine = true;
                        if (thickness == 0)
                            thickness = 1;
                    }
                } else {
                    // It's not already at the bottom edge, so crop the bottom edge.
                    cropBottom = true;

                    // Now that we've found the line, find out how thick it is.
                    for (int yy = y; yy >= qCeil(image->height() / 2.0); --yy) {
                        const QColor pixelColor = image->pixelColor(x, yy);
                        if (pixelColor == black || pixelColor == red) {
                            cropArea.setBottom(yy);
                        } else {
                            break;
                        }
                    }

                    // + 1 for the pixel that we leave in when cropping,
                    // another +1 for the fact that this else statement is only entered when y < image->height() - 1
                    if (thickness == 0) {
                        thickness = y - cropArea.bottom() + 2;
                        qCDebug(lcAssetFixerVerbose) << "found first croppable nine patch line on bottom edge at x" << x << "y" << y
                            << "with thickness" << thickness;
                    } else {
                        qCDebug(lcAssetFixerVerbose) << "found first croppable nine patch line on bottom edge at x" << x << "y" << y
                            << "using existing thickness of" << thickness;
                    }
                }
                break;
            }
        }
    }

    bool cropLeft = false;
    foundOnePixelThick9PatchLine = false;
    for (int x = 0; x < qFloor(image->width() / 2.0) && !cropLeft && !foundOnePixelThick9PatchLine; ++x) {
        for (int y = qMax(1, thickness); y < image->height() - 2 && !cropLeft && !foundOnePixelThick9PatchLine; ++y) {
            const QColor pixelColor = image->pixelColor(x, y);
            if (pixelColor == black || pixelColor == red) {
                if (x == 0) {
                    const QColor pixelColorToTheRight = image->pixelColor(x + 1, y);
                    if (pixelColorToTheRight != black && pixelColorToTheRight != red) {
                        // We've already found the beginning of the 9-patch line, and the column after it
                        // is a different color, so we know that it's one pixel thick, and that we're done.
                        qCDebug(lcAssetFixerVerbose) << "found one-pixel-thick nine patch line on left edge at y" << y;
                        foundOnePixelThick9PatchLine = true;
                    }
                } else {
                    // It's not already at the left edge, so crop the left edge.
                    cropLeft = true;

                    // Now that we've found the line, find out how thick it is.
                    for (int xx = x; xx < qFloor(image->width() / 2.0); ++xx) {
                        const QColor pixelColor = image->pixelColor(xx, y);
                        if (pixelColor == black || pixelColor == red) {
                            cropArea.setLeft(xx);
                        } else {
                            break;
                        }
                    }

                    // + 1 for the pixel that we leave in when cropping,
                    // another +1 for the fact that this else statement is only entered when x > 0
                    if (thickness == 0) {
                        thickness = cropArea.left() - x + 2;
                        qCDebug(lcAssetFixerVerbose) << "found first croppable nine patch line on left edge at x" << x << "y" << y
                            << "with thickness" << thickness;
                    } else {
                        qCDebug(lcAssetFixerVerbose) << "found first croppable nine patch line on left edge at x" << x << "y" << y
                            << "using existing thickness of" << thickness;
                    }
                }
            }
        }
    }

    bool cropRight = false;
    foundOnePixelThick9PatchLine = false;
    for (int x = image->width() - 1; x >= qCeil(image->width() / 2.0) && !cropRight && !foundOnePixelThick9PatchLine; --x) {
        for (int y = qMax(1, thickness); y < image->height() - 2 && !cropRight && !foundOnePixelThick9PatchLine; ++y) {
            const QColor pixelColor = image->pixelColor(x, y);
            if (pixelColor == black || pixelColor == red) {
                if (x == image->width() - 1) {
                    const QColor pixelColorToTheLeft = image->pixelColor(x - 1, y);
                    if (pixelColorToTheLeft != black && pixelColorToTheLeft != red) {
                        // We've already found the end of the 9-patch line, and the column before it
                        // is a different color, so we know that it's one pixel thick, and that we're done.
                        qCDebug(lcAssetFixerVerbose) << "found one-pixel-thick nine patch line on right edge at y" << y;
                        foundOnePixelThick9PatchLine = true;
                    }
                } else {
                    // It's not already at the right edge, so crop the right edge.
                    cropRight = true;

                    // Now that we've found the line, find out how thick it is.
                    for (int xx = x; xx >= qCeil(image->width() / 2.0); --xx) {
                        const QColor pixelColor = image->pixelColor(xx, y);
                        if (pixelColor == black || pixelColor == red) {
                            cropArea.setRight(xx);
                        } else {
                            break;
                        }
                    }

                    // + 1 for the pixel that we leave in when cropping,
                    // another +1 for the fact that this else statement is only entered when x < image->width() - 1
                    if (thickness == 0) {
                        thickness = x - cropArea.right() + 2;
                        qCDebug(lcAssetFixerVerbose) << "found first croppable nine patch line on right edge at x" << x << "y" << y
                            << "with thickness" << thickness;
                    } else {
                        qCDebug(lcAssetFixerVerbose) << "found first croppable nine patch line on right edge at x" << x << "y" << y
                            << "using existing thickness of" << thickness;
                    }
                }
                break;
            }
        }
    }

    const QRect copyArea(cropLeft ? cropArea.x() : (thickness ? thickness - 1 : 0),
        cropTop ? cropArea.y() : (thickness ? thickness - 1 : 0),
        cropRight ? cropArea.width() : image->width() - (thickness ? (thickness - 1) * 2 : 0),
        cropBottom ? cropArea.height() : image->height() - (thickness ? (thickness - 1) * 2 : 0));

    if (cropLeft | cropRight | cropTop | cropBottom) {
        qCDebug(lcAssetFixerVerbose) << "cropping area" << copyArea;
        *image = image->copy(copyArea);
        return true;
    }

    return false;
}

AssetFixer::AssetFixer(QObject *parent) :
    QObject(parent),
    mComponentComplete(false),
    mFirstWatch(true),
    mShouldWatch(false),
    mShouldFix(false),
    mLastModified(QDateTime::fromSecsSinceEpoch(0))
{
}

bool AssetFixer::shouldWatch() const
{
    return mShouldWatch;
}

void AssetFixer::setShouldWatch(bool watch)
{
    if (watch == mShouldWatch)
        return;

    stopWatching();

    mShouldWatch = watch;

    startWatching();

    emit shouldWatchChanged();
}

bool AssetFixer::shouldFix() const
{
    return mShouldFix;
}

void AssetFixer::setShouldFix(bool fix)
{
    if (fix == mShouldFix)
        return;

    mShouldFix = fix;
    emit shouldFixChanged();
}

QString AssetFixer::assetDirectory() const
{
    return mAssetDirectory;
}

void AssetFixer::setAssetDirectory(const QString &assetDirectory)
{
    if (assetDirectory == mAssetDirectory)
        return;

    stopWatching();

    const QString oldAssetDirectory = assetDirectory;
    mAssetDirectory.clear();

    if (isAssetDirectoryValid(assetDirectory)) {
        mAssetDirectory = assetDirectory;
        startWatching();
    }

    if (mAssetDirectory != oldAssetDirectory)
        emit assetDirectoryChanged();
}

QUrl AssetFixer::assetDirectoryUrl() const
{
    return QUrl::fromLocalFile(mAssetDirectory);
}

QDateTime AssetFixer::assetDirectoryLastModified() const
{
    return mLastModified;
}

void AssetFixer::setAssetDirectoryLastModified(const QDateTime &assetDirectoryLastModified)
{
    if (assetDirectoryLastModified == mLastModified)
        return;

    mLastModified = assetDirectoryLastModified;
    emit assetDirectoryLastModifiedChanged();
}

void AssetFixer::componentComplete()
{
    mComponentComplete = true;
}

void AssetFixer::classBegin()
{
}

void AssetFixer::onAssetsChanged()
{
    const QFileInfo fileInfo(mAssetDirectory);
    const QDateTime lastModified = fileInfo.lastModified();

    qCDebug(lcAssetFixer) << "Change in asset directory" << mAssetDirectory << "detected"
                          << "lastModified:" << lastModified;
    const qint64 secsSinceLastModification = mLastModified.secsTo(lastModified);
    if (secsSinceLastModification == 0) {
        qCDebug(lcAssetFixer) << "Change in asset directory" << mAssetDirectory << "detected, "
            << "but QFileInfo says the directory hasn't been modified; ignoring";
    } else {
        setAssetDirectoryLastModified(lastModified);

        QString message;
        if (lcAssetFixer().isDebugEnabled()) {
            message = QString::fromLatin1("Change in asset directory %1 detected, and QFileInfo says that there have been " \
                "%2 seconds since it was previously last modified); %3").arg(mAssetDirectory).arg(secsSinceLastModification);
        }

        if (shouldFix()) {
            qCDebug(lcAssetFixer) << message.arg(QLatin1String("suggesting delayed fix"));
            emit delayedFixSuggested();
        } else {
            qCDebug(lcAssetFixer) << message.arg(QLatin1String("suggesting reload"));
            emit reloadSuggested();
        }
    }
}

void AssetFixer::stopWatching()
{
    if (!mShouldWatch || mAssetDirectory.isEmpty() || !mComponentComplete)
        return;

    disconnect(&mFileSystemWatcher, &QFileSystemWatcher::directoryChanged, this, &AssetFixer::onAssetsChanged);
    mFileSystemWatcher.removePath(mAssetDirectory);
}

void AssetFixer::startWatching()
{
    if (!mShouldWatch || mAssetDirectory.isEmpty() || !mComponentComplete || !isAssetDirectoryValid(mAssetDirectory))
        return;

    if (mFileSystemWatcher.addPath(mAssetDirectory)) {
        // TODO: for some reason this is not called when an image is edited, but is when the same image is "touch"ed.
        // We could add watchers for each file, but then the application might have to be limited to displaying
        // the elements for one control at a time so that we don't breach the 256 file descriptor limit on some platforms:
        // http://doc.qt.io/qt-5/qfilesystemwatcher.html#details

        // We only emit a signal here rather than automatically responding to it ourselves,
        // because we want to give the UI time to start animations.
        connect(&mFileSystemWatcher, &QFileSystemWatcher::directoryChanged, this, &AssetFixer::onAssetsChanged);

        const QFileInfo fileInfo(mAssetDirectory);
        bool suggestFix = false;
        if (mFirstWatch) {
            mFirstWatch = false;

            // Here we check if the assets have been modified since the last time the application closed.
            // Checking this avoids a slow startup (due to fixing up assets).
            if (fileInfo.lastModified() > mLastModified) {
                qCDebug(lcAssetFixer) << "asset directory" << mAssetDirectory << "was modified at"
                    << fileInfo.lastModified() << ", which is later than our last stored modification time of"
                    << mLastModified << "; suggesting fix";
                suggestFix = true;
            } else {
                qCDebug(lcAssetFixer) << "asset directory" << mAssetDirectory << "has not been modified since"
                    << "the application was last closed; a fix is not necessary";

                // For some reason not all assets are updated if we don't do this.
                emit reloadSuggested();
            }

            // Don't need to call setAssetDirectoryLastModified() here, as we should have gotten it from settings.
        } else {
            suggestFix = true;
        }

        if (suggestFix) {
            setAssetDirectoryLastModified(fileInfo.lastModified());
            emit fixSuggested();
        }
    } else {
        qWarning() << "Could not watch asset directory" << mAssetDirectory;
    }
}

bool AssetFixer::isAssetDirectoryValid(const QString &assetDirectory)
{
    DirectoryValidator validator;
    validator.setPath(assetDirectory);
    return validator.isValid();
}

void AssetFixer::clearImageCache()
{
    QQmlApplicationEngine *engine = qobject_cast<QQmlApplicationEngine*>(qmlEngine(this));
    if (!engine) {
        qWarning() << "No QQmlApplicationEngine for AssetFixer - assets may not reload properly";
        return;
    }

    QQuickWindow *window = qobject_cast<QQuickWindow*>(engine->rootObjects().first());
    if (!window) {
        qWarning() << "No QQuickWindow - assets may not reload properly";
        return;
    }

    // We can't seem to disable image caching on a per-Image basis (by the time the QQuickImages
    // are available, the cache has already been filled), so we call this instead.
    qCDebug(lcAssetFixer) << "Calling QQuickWindow::releaseResources() to clear pixmap cache";
    window->releaseResources();
}

void AssetFixer::fixAssets()
{
    if (!mShouldFix || !mComponentComplete || mAssetDirectory.isEmpty() || !isAssetDirectoryValid(mAssetDirectory))
        return;

    QDir assetDir(mAssetDirectory);
    qCDebug(lcAssetFixer) << "Fixing up assets in" << assetDir.absolutePath() << "...";
    int filesChanged = 0;

    QStringList nameFilters;
    nameFilters << QLatin1String("*.9.png");
    QDirIterator dirIt(assetDir.absolutePath(), nameFilters, QDir::Files | QDir::Readable | QDir::NoSymLinks);
    while (dirIt.hasNext()) {
        const QString imagePath = dirIt.next();

        QImage image(imagePath);
        if (image.isNull()) {
            qWarning() << "Couldn't open image at" << imagePath;
            return;
        }

        qCDebug(lcAssetFixerVerbose).nospace() << "found " << imagePath << " (" << image.width() << "x" << image.height() << ") - "
            << "checking if we need to crop 9-patch lines";

        if (cropImageToLines(&image)) {
            if (!image.save(imagePath)) {
                qWarning() << "Couldn't save" << imagePath;
                return;
            }

            ++filesChanged;
        }
    }

    qCDebug(lcAssetFixer) << "Fixed" << filesChanged << "assets";

    // Let the application know that it should reload the Imagine style's assets.
    // Currently we always suggest a reload after fixing files, even if no files were fixed.
    // This is because the default Imagine style assets are automatically loaded at first, and then we
    // set a custom path shortly after, so we must ensure that the Imagine style is using the correct assets.
    // Reloads are just a matter of changing Imagine.path, which is very fast.
    emit reloadSuggested();
}
