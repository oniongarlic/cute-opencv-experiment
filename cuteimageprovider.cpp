#include "cuteimageprovider.h"

#include <QDebug>

#include <cmath>

CuteImageProvider::CuteImageProvider(QObject *parent) :
    QObject (parent),
    QQuickImageProvider(QQuickImageProvider::Image)
{
    qDebug() << "CuteImageProvider";
}

CuteImageProvider::~CuteImageProvider()
{
    qDebug() << "~CuteImageProvider";
    m_image=QImage();
    m_modified=QImage();
}

bool CuteImageProvider::setImage(QString file)
{
    QMutexLocker lock(&mutex);
    if (file.startsWith("file://"))
        file.remove(0,7);

    qDebug() << "setImageQS" << file;
    m_modified=QImage();

    return m_image.load(file);
}

bool CuteImageProvider::setImage(QImage &image)
{
    QMutexLocker lock(&mutex);

    qDebug() << "setImageQI" << image.format();

    m_image=image;
    m_modified=QImage();

    return true;
}

bool CuteImageProvider::setImage(QVariant image)
{
    QMutexLocker lock(&mutex);

    qDebug() << "setImageQV" << image.typeName() << image.type();

    m_modified=QImage();

    switch ((QMetaType::Type)image.type()) {
    case QMetaType::QUrl: {
        QUrl tmp=image.value<QUrl>();
        qDebug() << "QUrl" << tmp << tmp.path();
        m_image.load(tmp.path());
    }
        break;
    case QMetaType::QImage: {
        QImage tmp=image.value<QImage>();
        m_image=tmp.rgbSwapped(); // XXX Why do we need this ?        
    }
        break;
    default:
        return false;
    }

    return m_image.isNull();
}

bool CuteImageProvider::isEmpty() const
{
    return m_image.isNull();
}

void CuteImageProvider::clear()
{
    QMutexLocker lock(&mutex);

    m_image=QImage();
    m_modified=QImage();

    lock.unlock();
    emit imageChanged();
}

void CuteImageProvider::reset()
{
    QMutexLocker lock(&mutex);

    m_modified=QImage();

    lock.unlock();
    emit imageChanged();
}

void CuteImageProvider::commit()
{
    QMutexLocker lock(&mutex);

    if (!m_modified.isNull())
        m_image=m_modified;

    lock.unlock();
    emit imageChanged();
}

void CuteImageProvider::mirror(bool horizontal, bool vertical)
{
    QMutexLocker lock(&mutex);

    m_modified=m_image.mirrored(horizontal, vertical);
}

void CuteImageProvider::cropNormalized(QRectF rect)
{
    QRect mapped(qRound(rect.x()*m_image.width()),
                 qRound(rect.y()*m_image.height()),
                 qRound(rect.width()*m_image.width()),
                 qRound(rect.height()*m_image.height()));

    qDebug() << mapped;

    crop(mapped);
}

void CuteImageProvider::crop(QRect &rect)
{
    QMutexLocker lock(&mutex);
    m_modified=m_image.copy(rect);

    lock.unlock();
    emit imageChanged();
}

void CuteImageProvider::adjustContrastBrightness(double contrast, double brightness)
{
    QMutexLocker lock(&mutex);
    unsigned char lut[256];

    prepareImage();

    int width = m_modified.width();
    int height = m_modified.height();

    double factor = (255.0 + contrast) / 255.0;

    for (int i=0;i<256;i++) {
        double l=(factor * (i - 128.0)) + 128.0;
        l+=l*brightness;
        lut[i]=qRound(qBound(0.0, l, 255.0));
    }

    for (int y=0;y<height;y++) {
        uchar *sl=m_modified.scanLine(y);
        for (int x=0;x<width;x++) {
            QRgb* p = reinterpret_cast<QRgb*>(sl+x*4);

            int r=qRed(*p);
            int g=qGreen(*p);
            int b=qBlue(*p);

            *p=qRgb(lut[r], lut[g], lut[b]);
#if 0
            double r=qRed(*p);
            double g=qGreen(*p);
            double b=qBlue(*p);

            // Contrast
            r=(factor * (r - 128.0)) + 128.0;
            g=(factor * (g - 128.0)) + 128.0;
            b=(factor * (b - 128.0)) + 128.0;

            // Brightness
            r+=r*brightness;
            g+=g*brightness;
            b+=b*brightness;

            r=qBound(0.0, r, 255.0);
            g=qBound(0.0, g, 255.0);
            b=qBound(0.0, b, 255.0);

            *p=qRgb(qRound(r),qRound(g),qRound(b));
#endif
        }
    }

    lock.unlock();
    emit imageChanged();
}

void CuteImageProvider::gamma(double gamma)
{
    QMutexLocker lock(&mutex);
    unsigned char lut[256];

    prepareImage();

    int width = m_modified.width();
    int height = m_modified.height();

    for( int i = 0; i < 256; ++i)
        lut[i]=qRound(qBound(0.0, std::pow(i / 255.0, gamma) * 255.0, 255.0));

    for (int y=0;y<height;y++) {
        uchar *sl=m_modified.scanLine(y);
        for (int x=0;x<width;x++) {
            QRgb* p = reinterpret_cast<QRgb*>(sl+x*4);

            int r=qRed(*p);
            int g=qGreen(*p);
            int b=qBlue(*p);

            *p=qRgb(lut[r], lut[g], lut[b]);
        }
    }
    lock.unlock();
    emit imageChanged();
}

void CuteImageProvider::gray()
{
    QMutexLocker lock(&mutex);

    prepareImage();

    int width = m_modified.width();
    int height = m_modified.height();

    for (int y=0;y<height;y++) {
        uchar *sl=m_modified.scanLine(y);
        for (int x=0;x<width;x++) {
            QRgb* p = reinterpret_cast<QRgb*>(sl+x*4);
            int g=qGray(*p);
            *p=qRgb(g,g,g);
        }
    }
}

void CuteImageProvider::prepareImage()
{
    if (m_image.format()!=QImage::Format_RGB32)
        m_modified=m_image.convertToFormat(QImage::Format_RGB32);
    else {
        m_modified=m_image;
    }
    m_modified.detach();
}

void CuteImageProvider::rotate(double angle, bool smooth)
{
    QMutexLocker lock(&mutex);

    QPoint center = m_image.rect().center();

    QMatrix matrix;
    matrix.translate(center.x(), center.y());
    matrix.rotate(angle);

    m_modified = m_image.transformed(matrix, smooth ? Qt::SmoothTransformation : Qt::FastTransformation);
}

bool CuteImageProvider::save(QString fileName)
{
    QMutexLocker lock(&mutex);

    if (fileName.startsWith("file://"))
        fileName.remove(0,7);

    return m_image.save(fileName);
}

QImage CuteImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    QImage r;
    QMutexLocker lock(&mutex);

    qDebug() << id << size << requestedSize;

    int width = m_image.width();
    int height = m_image.height();

    if (size)
        *size = QSize(width, height);

    if (id=="preview" && !m_modified.isNull()) {
        r=m_modified;
    } else {
        r=m_image;
    }

    if (requestedSize.width() >0 && requestedSize.height() > 0) {
        return r.scaled(requestedSize.width(), requestedSize.height());
    }

    return r;
}
