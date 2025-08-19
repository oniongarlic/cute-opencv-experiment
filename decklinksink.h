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
    Q_PROPERTY(QObject* videoSink WRITE setVideoSink FINAL)
    Q_PROPERTY(bool haveDeckLink READ haveDeckLink NOTIFY haveDeckLinkChanged FINAL)
    Q_PROPERTY(int devices READ devices NOTIFY devicesChanged FINAL)
public:
    explicit Decklinksink(QObject *parent = nullptr);
    bool haveDeckLink() const;

    Q_INVOKABLE void setFramebufferSize(QSize size);
    Q_INVOKABLE void setVideoSink(QObject *videosink);

    Q_INVOKABLE bool setOutput(uint index);

    int devices() const;

signals:
    void haveDeckLinkChanged();
    void devicesChanged();

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

    QVideoSink *m_videosink=nullptr;

    // Available outputs
    QList<IDeckLinkOutput *> m_outputs;

    // Active output and frame
    IDeckLinkOutput *m_output=nullptr;
    IDeckLinkMutableVideoFrame* m_frame=nullptr;
};

#endif // DECKLINKSINK_H
