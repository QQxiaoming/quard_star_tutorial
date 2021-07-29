/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtQuick module of the Qt Toolkit.
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

#include "qquickanimatedimage_p.h"
#include "qquickanimatedimage_p_p.h"

#include <QtGui/qguiapplication.h>
#include <QtQml/qqmlinfo.h>
#include <QtQml/qqmlfile.h>
#include <QtQml/qqmlengine.h>
#include <QtGui/qmovie.h>
#if QT_CONFIG(qml_network)
#include <QtNetwork/qnetworkrequest.h>
#include <QtNetwork/qnetworkreply.h>
#endif

QT_BEGIN_NAMESPACE

QQuickPixmap* QQuickAnimatedImagePrivate::infoForCurrentFrame(QQmlEngine *engine)
{
    if (!movie)
        return nullptr;

    int current = movie->currentFrameNumber();
    if (!frameMap.contains(current)) {
        QUrl requestedUrl;
        QQuickPixmap *pixmap = nullptr;
        if (engine && !movie->fileName().isEmpty()) {
            requestedUrl.setUrl(QString::fromUtf8("quickanimatedimage://%1#%2")
                                .arg(movie->fileName())
                                .arg(current));
        }
        if (!requestedUrl.isEmpty()) {
            if (QQuickPixmap::isCached(requestedUrl, QSize(), QQuickImageProviderOptions()))
                pixmap = new QQuickPixmap(engine, requestedUrl);
            else
                pixmap = new QQuickPixmap(requestedUrl, movie->currentImage());
        } else {
            pixmap = new QQuickPixmap;
            pixmap->setImage(movie->currentImage());
        }
        frameMap.insert(current, pixmap);
    }

    return frameMap.value(current);
}

/*!
    \qmltype AnimatedImage
    \instantiates QQuickAnimatedImage
    \inqmlmodule QtQuick
    \inherits Image
    \brief Plays animations stored as a series of images.
    \ingroup qtquick-visual

    The AnimatedImage type extends the features of the \l Image type, providing
    a way to play animations stored as images containing a series of frames,
    such as those stored in GIF files.

    Information about the current frame and total length of the animation can be
    obtained using the \l currentFrame and \l frameCount properties. You can
    start, pause and stop the animation by changing the values of the \l playing
    and \l paused properties.

    The full list of supported formats can be determined with QMovie::supportedFormats().

    \section1 Example Usage

    \beginfloatleft
    \image animatedimageitem.gif
    \endfloat

    The following QML shows how to display an animated image and obtain information
    about its state, such as the current frame and total number of frames.
    The result is an animated image with a simple progress indicator underneath it.

    \b Note: When animated images are cached, every frame of the animation will be cached.

    Set cache to false if you are playing a long or large animation and you
    want to conserve memory.

    If the image data comes from a sequential device (e.g. a socket),
    AnimatedImage can only loop if cache is set to true.

    \clearfloat
    \snippet qml/animatedimage.qml document

    \sa BorderImage, Image
*/

/*!
    \qmlproperty url QtQuick::AnimatedImage::source

    This property holds the URL that refers to the source image.

    AnimatedImage can handle any image format supported by Qt, loaded from any
    URL scheme supported by Qt.

    \sa QQuickImageProvider
*/

QQuickAnimatedImage::QQuickAnimatedImage(QQuickItem *parent)
    : QQuickImage(*(new QQuickAnimatedImagePrivate), parent)
{
    connect(this, &QQuickImageBase::cacheChanged, this, &QQuickAnimatedImage::onCacheChanged);
}

QQuickAnimatedImage::~QQuickAnimatedImage()
{
    Q_D(QQuickAnimatedImage);
#if QT_CONFIG(qml_network)
    if (d->reply)
        d->reply->deleteLater();
#endif
    delete d->movie;
    qDeleteAll(d->frameMap);
    d->frameMap.clear();
}

/*!
  \qmlproperty bool QtQuick::AnimatedImage::paused
  This property holds whether the animated image is paused.

  By default, this property is false. Set it to true when you want to pause
  the animation.
*/

bool QQuickAnimatedImage::isPaused() const
{
    Q_D(const QQuickAnimatedImage);
    if (!d->movie)
        return d->paused;
    return d->movie->state()==QMovie::Paused;
}

void QQuickAnimatedImage::setPaused(bool pause)
{
    Q_D(QQuickAnimatedImage);
    if (pause == d->paused)
        return;
    if (!d->movie) {
        d->paused = pause;
        emit pausedChanged();
    } else {
        d->movie->setPaused(pause);
    }
}

/*!
  \qmlproperty bool QtQuick::AnimatedImage::playing
  This property holds whether the animated image is playing.

  By default, this property is true, meaning that the animation
  will start playing immediately.

  \b Note: this property is affected by changes to the actual playing
  state of AnimatedImage. If non-animated images are used, \a playing
  will need to be manually set to \a true in order to animate
  following images.
  \qml
  AnimatedImage {
      onStatusChanged: playing = (status == AnimatedImage.Ready)
  }
  \endqml
*/

bool QQuickAnimatedImage::isPlaying() const
{
    Q_D(const QQuickAnimatedImage);
    if (!d->movie)
        return d->playing;
    return d->movie->state()!=QMovie::NotRunning;
}

void QQuickAnimatedImage::setPlaying(bool play)
{
    Q_D(QQuickAnimatedImage);
    if (play == d->playing)
        return;
    if (!d->movie) {
        d->playing = play;
        emit playingChanged();
        return;
    }
    if (play)
        d->movie->start();
    else
        d->movie->stop();
}

/*!
  \qmlproperty int QtQuick::AnimatedImage::currentFrame
  \qmlproperty int QtQuick::AnimatedImage::frameCount

  currentFrame is the frame that is currently visible. By monitoring this property
  for changes, you can animate other items at the same time as the image.

  frameCount is the number of frames in the animation. For some animation formats,
  frameCount is unknown and has a value of zero.
*/
int QQuickAnimatedImage::currentFrame() const
{
    Q_D(const QQuickAnimatedImage);
    if (!d->movie)
        return d->presetCurrentFrame;
    return d->movie->currentFrameNumber();
}

void QQuickAnimatedImage::setCurrentFrame(int frame)
{
    Q_D(QQuickAnimatedImage);
    if (!d->movie) {
        d->presetCurrentFrame = frame;
        return;
    }
    d->movie->jumpToFrame(frame);
}

int QQuickAnimatedImage::frameCount() const
{
    Q_D(const QQuickAnimatedImage);
    if (!d->movie)
        return 0;
    return d->movie->frameCount();
}

/*!
    \qmlproperty real QtQuick::AnimatedImage::speed
    \since QtQuick 2.11

    This property holds the speed of the animation.

    The speed is measured in percentage of the original animated image speed.
    The default speed is 1.0 (original speed).
*/
qreal QQuickAnimatedImage::speed() const
{
    Q_D(const QQuickAnimatedImage);
    return d->speed;
}

void QQuickAnimatedImage::setSpeed(qreal speed)
{
    Q_D(QQuickAnimatedImage);
    if (d->speed != speed) {
        d->speed = speed;
        if (d->movie)
            d->movie->setSpeed(qRound(speed * 100.0));
        emit speedChanged();
    }
}

void QQuickAnimatedImage::setSource(const QUrl &url)
{
    Q_D(QQuickAnimatedImage);
    if (url == d->url)
        return;

#if QT_CONFIG(qml_network)
    if (d->reply) {
        d->reply->deleteLater();
        d->reply = nullptr;
    }
#endif

    d->setImage(QImage());
    qDeleteAll(d->frameMap);
    d->frameMap.clear();

    d->oldPlaying = isPlaying();
    d->setMovie(nullptr);
    d->url = url;
    emit sourceChanged(d->url);

    if (isComponentComplete())
        load();
}

void QQuickAnimatedImage::load()
{
    Q_D(QQuickAnimatedImage);

    if (d->url.isEmpty()) {
        if (d->progress != 0) {
            d->progress = 0;
            emit progressChanged(d->progress);
        }

        d->setImage(QImage());
        d->status = Null;
        emit statusChanged(d->status);

        d->currentSourceSize = QSize(0, 0);
        if (d->currentSourceSize != d->oldSourceSize) {
            d->oldSourceSize = d->currentSourceSize;
            emit sourceSizeChanged();
        }
        if (isPlaying() != d->oldPlaying)
            emit playingChanged();
    } else {
        const qreal targetDevicePixelRatio = (window() ? window()->effectiveDevicePixelRatio() : qApp->devicePixelRatio());
        d->devicePixelRatio = 1.0;

        QUrl loadUrl = d->url;
        resolve2xLocalFile(d->url, targetDevicePixelRatio, &loadUrl, &d->devicePixelRatio);
        QString lf = QQmlFile::urlToLocalFileOrQrc(loadUrl);

        if (!lf.isEmpty()) {
            d->setMovie(new QMovie(lf));
            movieRequestFinished();
        } else {
#if QT_CONFIG(qml_network)
            if (d->status != Loading) {
                d->status = Loading;
                emit statusChanged(d->status);
            }
            if (d->progress != 0) {
                d->progress = 0;
                emit progressChanged(d->progress);
            }
            QNetworkRequest req(d->url);
            req.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);

            d->reply = qmlEngine(this)->networkAccessManager()->get(req);
            connect(d->reply, &QNetworkReply::finished, this, &QQuickAnimatedImage::movieRequestFinished);
            connect(d->reply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(requestProgress(qint64,qint64)));
#endif
        }
    }
}

#define ANIMATEDIMAGE_MAXIMUM_REDIRECT_RECURSION 16

void QQuickAnimatedImage::movieRequestFinished()
{

    Q_D(QQuickAnimatedImage);

#if QT_CONFIG(qml_network)
    if (d->reply) {
        d->redirectCount++;
        if (d->redirectCount < ANIMATEDIMAGE_MAXIMUM_REDIRECT_RECURSION) {
            QVariant redirect = d->reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
            if (redirect.isValid()) {
                QUrl url = d->reply->url().resolved(redirect.toUrl());
                d->reply->deleteLater();
                setSource(url);
                return;
            }
        }

        d->redirectCount=0;
        d->setMovie(new QMovie(d->reply));
    }
#endif

    if (!d->movie || !d->movie->isValid()) {
        qmlWarning(this) << "Error Reading Animated Image File " << d->url.toString();
        d->setMovie(nullptr);
        d->setImage(QImage());
        if (d->progress != 0) {
            d->progress = 0;
            emit progressChanged(d->progress);
        }
        d->status = Error;
        emit statusChanged(d->status);

        d->currentSourceSize = QSize(0, 0);
        if (d->currentSourceSize != d->oldSourceSize) {
            d->oldSourceSize = d->currentSourceSize;
            emit sourceSizeChanged();
        }
        if (isPlaying() != d->oldPlaying)
            emit playingChanged();
        return;
    }

    connect(d->movie, &QMovie::stateChanged, this, &QQuickAnimatedImage::playingStatusChanged);
    connect(d->movie, &QMovie::frameChanged, this, &QQuickAnimatedImage::movieUpdate);
    if (d->cache)
        d->movie->setCacheMode(QMovie::CacheAll);
    d->movie->setSpeed(qRound(d->speed * 100.0));

    d->status = Ready;
    emit statusChanged(d->status);

    if (d->progress != 1.0) {
        d->progress = 1.0;
        emit progressChanged(d->progress);
    }

    bool pausedAtStart = d->paused;
    if (d->movie && d->playing)
        d->movie->start();
    if (d->movie && pausedAtStart)
        d->movie->setPaused(true);
    if (d->movie && (d->paused || !d->playing)) {
        d->movie->jumpToFrame(d->presetCurrentFrame);
        d->presetCurrentFrame = 0;
    }

    QQuickPixmap *pixmap = d->infoForCurrentFrame(qmlEngine(this));
    if (pixmap)
        d->setPixmap(*pixmap);

    if (isPlaying() != d->oldPlaying)
        emit playingChanged();

    if (d->movie)
        d->currentSourceSize = d->movie->currentPixmap().size();
    else
        d->currentSourceSize = QSize(0, 0);

    if (d->currentSourceSize != d->oldSourceSize) {
        d->oldSourceSize = d->currentSourceSize;
        emit sourceSizeChanged();
    }
}

void QQuickAnimatedImage::movieUpdate()
{
    Q_D(QQuickAnimatedImage);

    if (!d->cache) {
        qDeleteAll(d->frameMap);
        d->frameMap.clear();
    }

    if (d->movie) {
        d->setPixmap(*d->infoForCurrentFrame(qmlEngine(this)));
        emit frameChanged();
    }
}

void QQuickAnimatedImage::playingStatusChanged()
{
    Q_D(QQuickAnimatedImage);

    if ((d->movie->state() != QMovie::NotRunning) != d->playing) {
        d->playing = (d->movie->state() != QMovie::NotRunning);
        emit playingChanged();
    }
    if ((d->movie->state() == QMovie::Paused) != d->paused) {
        d->paused = (d->movie->state() == QMovie::Paused);
        emit pausedChanged();
    }
}

void QQuickAnimatedImage::onCacheChanged()
{
    Q_D(QQuickAnimatedImage);
    if (!cache()) {
        qDeleteAll(d->frameMap);
        d->frameMap.clear();
        if (d->movie)
            d->movie->setCacheMode(QMovie::CacheNone);
    } else {
        if (d->movie)
            d->movie->setCacheMode(QMovie::CacheAll);
    }
}

QSize QQuickAnimatedImage::sourceSize()
{
    Q_D(QQuickAnimatedImage);
    return d->currentSourceSize;
}

void QQuickAnimatedImage::componentComplete()
{
    QQuickItem::componentComplete(); // NOT QQuickImage
    load();
}

void QQuickAnimatedImagePrivate::setMovie(QMovie *m)
{
    if (movie == m)
        return;
    Q_Q(QQuickAnimatedImage);
    const int oldFrameCount = q->frameCount();

    if (movie) {
        movie->disconnect();
        movie->deleteLater();
    }
    movie = m;

    if (oldFrameCount != q->frameCount())
        emit q->frameCountChanged();
}

QT_END_NAMESPACE

#include "moc_qquickanimatedimage_p.cpp"
