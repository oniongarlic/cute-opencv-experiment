#ifndef CUTEIMAGEPROVIDER_H
#define CUTEIMAGEPROVIDER_H

#include <QObject>
#include <QQuickImageProvider>
#include <QMutexLocker>
#include <QMutex>

class CuteImageProvider : public QObject, public QQuickImageProvider
{
    Q_OBJECT
public:
    CuteImageProvider(QObject *parent=nullptr);
    ~CuteImageProvider();

    Q_INVOKABLE bool setImage(QString file);
    Q_INVOKABLE bool setImage(QImage &image);
    Q_INVOKABLE bool setImage(QVariant image);
    Q_INVOKABLE bool isEmpty() const;

    Q_INVOKABLE void clear();
    Q_INVOKABLE void reset();

    Q_INVOKABLE void commit();

    Q_INVOKABLE void mirror(bool horizontal, bool vertical);

    Q_INVOKABLE void cropNormalized(QRectF rect);
    Q_INVOKABLE void crop(QRect &rect);

    Q_INVOKABLE void adjustContrastBrightness(double contrast, double brightness);        
    Q_INVOKABLE void gamma(double gamma);
    Q_INVOKABLE void gray();

    Q_INVOKABLE void rotate(double angle, bool smooth=true);

    Q_INVOKABLE bool save(QString fileName);

    // QQuickImageProvider interface
    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize);

signals:
    void imageChanged();

private:
    void prepareImage();

    QImage m_image;
    QImage m_modified;
    QMutex mutex;

};

#endif // CUTEIMAGEPROVIDER_H
