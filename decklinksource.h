#ifndef DECKLINKSOURCEK_H
#define DECKLINKSOURCEK_H

#include <QObject>
#include <QVariant>
#include <QImage>
#include <QVideoFrame>
#include <QQmlEngine>
#include <QtMultimedia/QVideoSink>
#include <QtMultimedia/QVideoFrame>

#include "DeckLinkAPI.h"

#include "decklink.h"

class DeckLinkInputCallback;

class Decklinksource: public QObject
{
    friend DeckLinkInputCallback;
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QObject* decklink READ getDecklink WRITE setDecklink NOTIFY decklinkChanged FINAL REQUIRED)    
public:
    explicit Decklinksource(QObject *parent = nullptr);
    bool haveDeckLink() const;

    Q_INVOKABLE void setFramebufferSize(QSize size);
    Q_INVOKABLE void setVideoSink(QObject *videosink);

    Q_INVOKABLE bool setInput(uint index);

    Q_INVOKABLE bool setMode(qint32 mode);

    Q_INVOKABLE bool setProfile(uint profile);
    Q_INVOKABLE bool setKeyer(bool enable);    

    QObject *getVideoSink() const;

    QObject *getDecklink() const;
    void setDecklink(QObject *newDecklink);

signals:
    void haveDeckLinkChanged();
    void devicesChanged();
    void decklinkChanged();

public slots:
    bool enableInput();
    bool disableInput();
protected:
    void imageToBuffer(const QImage &frame);
    void newFrame(QImage &frame);
private:
    DeckLink *m_decklink = nullptr;

    DeckLinkInputCallback *m_icb;

    int m_current=-1;
    QSize m_fbsize;
    QVideoSink *m_videosink=nullptr;

    // Active output/input/keyer and frame    
    IDeckLinkInput *m_input=nullptr;
    IDeckLinkKeyer *m_keyer=nullptr;
    IDeckLinkMutableVideoFrame* m_frame=nullptr;

    BMDDisplayMode m_mode=bmdModeHD1080p30; // bmdModeHD1080p6000; //
};

#endif // DECKLINKSOURCE_H
