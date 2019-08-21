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

    return m_image.load(file);
}

bool CuteImageProvider::setImage(QImage &image)
{
    QMutexLocker lock(&mutex);
    m_image=image;

    return true;
}

bool CuteImageProvider::isEmpty() const
{
    return m_image.isNull();
}

void CuteImageProvider::cropNormalized(QRectF rect)
{
    QRect mapped(qRound(rect.x()*m_image.width()),
                 qRound(rect.y()*m_image.height()),
                 qRound(rect.width()*m_image.width()),
                 qRound(rect.height()*m_image.height()));

    qDebug() << rect << mapped;

    crop(mapped);
}

void CuteImageProvider::crop(QRect &rect)
{
    QMutexLocker lock(&mutex);
    m_image=m_image.copy(rect);

    emit imageChanged();
}

bool CuteImageProvider::save(const QString &fileName)
{
    QMutexLocker lock(&mutex);
    return m_image.save(fileName);
}

QImage CuteImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    QMutexLocker lock(&mutex);
    qDebug() << id << size << requestedSize;

    int width = m_image.width();
    int height = m_image.height();

    if (size)
        *size = QSize(width, height);

    if (requestedSize.width() >0 && requestedSize.height() > 0) {
        return m_image.scaled(requestedSize.width(), requestedSize.height());
    }

    return m_image;
}
