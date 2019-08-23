#include "cuteimageprovider.h"

#include <QDebug>

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

    QImage tmp=image.value<QImage>();

    qDebug() << "setImageQV" << image.typeName() << tmp.format();

    // tmp.rgbSwapped();

    m_image=tmp.rgbSwapped(); // XXX Why do we need this ?
    m_modified=QImage();

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

void CuteImageProvider::cropNormalized(QRectF rect)
{
    QRect mapped(qRound(rect.x()*m_image.width()),
                 qRound(rect.y()*m_image.height()),
                 qRound(rect.width()*m_image.width()),
                 qRound(rect.height()*m_image.height()));

    crop(mapped);
}

void CuteImageProvider::crop(QRect &rect)
{
    QMutexLocker lock(&mutex);
    m_modified=m_image.copy(rect);
}

void CuteImageProvider::adjustContrastBrightness(double contrast, double brightness)
{
    int width = m_image.width();
    int height = m_image.height();

    if (m_image.format()!=QImage::Format_RGB32)
        m_modified=m_image.convertToFormat(QImage::Format_RGB32);
    else {
        m_modified=m_image;
    }
    m_modified.detach();

    qDebug() << contrast << brightness;

    for (int x=0;x<width;x++) {
        for (int y=0;y<height;y++) {
            QRgb p=m_modified.pixel(x,y);

            double r=qRed(p);
            double g=qGreen(p);
            double b=qBlue(p);

            // Contrast


            // Brightness
            r+=r*brightness;
            g+=g*brightness;
            b+=b*brightness;

            r=qBound(0.0,r,255.0);
            g=qBound(0.0,g,255.0);
            b=qBound(0.0,b,255.0);

            p=qRgb(qRound(r),qRound(g),qRound(b));

            m_modified.setPixel(x,y,p);
        }
    }
}

void CuteImageProvider::rotate(double angle)
{
    QMutexLocker lock(&mutex);

    QPoint center = m_image.rect().center();
    QMatrix matrix;
    matrix.translate(center.x(), center.y());
    matrix.rotate(angle);
    m_modified = m_image.transformed(matrix);
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
