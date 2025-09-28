#ifndef DECKLINKSINK_H
#define DECKLINKSINK_H

#include <QObject>
#include <QVariant>
#include <QImage>
#include <QVideoFrame>
#include <QQmlEngine>
#include <QtMultimedia/QVideoSink>
#include <QtMultimedia/QVideoFrame>

#include <QAudioBuffer>
#include <QAudioBufferOutput>
#include <QtMultimedia/QAudioSink>

#include <QIODevice>

#include "DeckLinkAPI.h"

#include "decklink.h"

class Decklinksink : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QObject* decklink READ getDecklink WRITE setDecklink NOTIFY decklinkChanged FINAL REQUIRED)
    Q_PROPERTY(QObject* videoSink READ getVideoSink WRITE setVideoSink NOTIFY videoSinkChanged FINAL)
    Q_PROPERTY(QObject* audioSink READ getAudioSink FINAL)
    Q_PROPERTY(bool keyEnabled READ keyEnabled NOTIFY keyEnabledChanged FINAL)
    Q_PROPERTY(bool audio READ audioEnabled WRITE setAudioEnabled NOTIFY audioEnabledChanged FINAL)
public:
    explicit Decklinksink(QObject *parent = nullptr);
    bool haveDeckLink() const;

    Q_INVOKABLE void setFramebufferSize(QSize size);
    Q_INVOKABLE void setVideoSink(QObject *videosink);

    Q_INVOKABLE bool setOutput(uint index);

    Q_INVOKABLE qint32 getMode();
    Q_INVOKABLE bool setMode(qint32 mode);

    Q_INVOKABLE bool setProfile(uint profile);
    Q_INVOKABLE bool setKeyer(bool enable, uint8_t level=255, bool external=true);

    Q_INVOKABLE bool keyerLevel(uint8_t level);
    Q_INVOKABLE bool keyerRampUp(uint32_t frames);
    Q_INVOKABLE bool keyerRampDown(uint32_t frames);

    QObject *getVideoSink() const;

    QObject *getDecklink() const;
    void setDecklink(QObject *newDecklink);

    bool keyEnabled() const;

    Q_INVOKABLE QObject *getAudioSink() const;
    Q_INVOKABLE QObject *getAudioBuffer() const;

    bool audioEnabled() const;
    void setAudioEnabled(bool newAudio);

signals:
    void haveDeckLinkChanged();
    void devicesChanged();
    void videoSinkChanged();

    void decklinkChanged();

    void keyEnabledChanged();

    void audioEnabledChanged();

public slots:
    void displayFrame(const QVideoFrame &frame);
    void displayImage(const QImage &frame);
    void displayImage(const QVariant image);

    bool enableOutput();
    bool disableOutput();

    void clearBuffer(int value=0);

protected slots:
    void onAudioBufferReceived(const QAudioBuffer &buffer);

protected:
    void imageToBuffer(const QImage &frame);
private:
    DeckLink *m_decklink = nullptr;
    int m_current=-1;
    QSize m_fbsize;
    QVideoSink *m_videosink=nullptr;
    QAudioSink *m_audiosink=nullptr;

    QIODevice *m_audiosinkdevice=nullptr;
    QAudioBufferOutput *m_audio_buffer=nullptr;
    bool m_audio=true;

    bool m_use_precompiled=true;
    bool m_keyEnabled=false;

    // Active output/input/keyer and frame
    IDeckLinkOutput *m_output=nullptr;
    IDeckLinkInput *m_input=nullptr;
    IDeckLinkKeyer *m_keyer=nullptr;
    IDeckLinkMutableVideoFrame* m_frame=nullptr;

    BMDDisplayMode m_mode=bmdModeHD1080p6000; // bmdModeHD1080p30    
};

#endif // DECKLINKSINK_H
