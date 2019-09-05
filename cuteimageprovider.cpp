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

    if (m_image.format()!=QImage::Format_RGB32)
        m_modified=m_image.convertToFormat(QImage::Format_RGB32);
    else {
        m_modified=m_image;
    }
    m_modified.detach();

    int width = m_modified.width();
    int height = m_modified.height();

    qDebug() << contrast << brightness;

    for (int x=0;x<width;x++) {
        for (int y=0;y<height;y++) {
            QRgb p=m_modified.pixel(x,y);

            double r=qRed(p);
            double g=qGreen(p);
            double b=qBlue(p);

            // Contrast
            double factor = (259.0 * (contrast + 255.0)) / (255.0 * (259.0 - contrast));
            r=(factor * (r - 128.0)) + 128.0;
            g=(factor * (g - 128.0)) + 128.0;
            g=(factor * (b - 128.0)) + 128.0;

            // Brightness
            r+=r*brightness;
            g+=g*brightness;
            b+=b*brightness;

            r=qBound(0.0, r, 255.0);
            g=qBound(0.0, g, 255.0);
            b=qBound(0.0, b, 255.0);

            p=qRgb(qRound(r),qRound(g),qRound(b));

            m_modified.setPixel(x,y,p);
        }
    }

    lock.unlock();
    emit imageChanged();
}

void CuteImageProvider::gray()
{
    int width = m_modified.width();
    int height = m_modified.height();

    for (int x=0;x<width;x++) {
        for (int y=0;y<height;y++) {
            QRgb p=m_modified.pixel(x,y);
            int g=qGray(p);
            m_modified.setPixel(x,y,qRgb(g,g,g));
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

    m_modified = m_image.transformed(matrix, Qt::SmoothTransformation);
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
