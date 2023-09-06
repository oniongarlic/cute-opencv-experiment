#ifndef DECKLINKSINK_H
#define DECKLINKSINK_H

#include <QObject>
#include <QVariant>
#include <QImage>
#include <QVideoFrame>
#include <QQmlEngine>
#include <QtMultimedia/QVideoSink>
#include <QtMultimedia/QVideoFrame>

#include "DeckLinkAPI.h"

class Decklinksink : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QObject* videoSink WRITE setVideoSink)
    Q_PROPERTY(bool haveDeckLink READ haveDeckLink NOTIFY haveDeckLinkChanged FINAL)
public:
    explicit Decklinksink(QObject *parent = nullptr);
    bool haveDeckLink() const;

signals:

    void haveDeckLinkChanged();

public slots:
    void displayFrame(const QVideoFrame &frame);
    void displayImage(const QImage &frame);
    void displayImage(const QVariant image);

    void enableOutput();
    void disableOutput();

    void clearBuffer();
protected:
    void imageToBuffer(const QImage &frame);
private:
    int m_devices=0;
    bool m_haveDeckLink=false;
    bool m_haveOutput=false;
    QSize m_fbsize;
    QVariantList m_deviceList;

    void setVideoSink(QObject *videosink);
    QVideoSink *m_videosink=nullptr;

    IDeckLinkOutput *m_output=nullptr;
    IDeckLinkMutableVideoFrame* m_frame=nullptr;
};

#endif // DECKLINKSINK_H
