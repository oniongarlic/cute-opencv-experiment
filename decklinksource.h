#ifndef DECKLINKSOURCEK_H
#define DECKLINKSOURCEK_H

#include <QObject>
#include <QVariant>
#include <QImage>
#include <QVideoFrame>
#include <QQueue>
#include <QMutex>
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
    Q_PROPERTY(QObject* videoSink READ getVideoSink WRITE setVideoSink NOTIFY videoSinkChanged FINAL)
    Q_PROPERTY(bool streaming READ streaming NOTIFY streamingChanged FINAL)
public:
    explicit Decklinksource(QObject *parent = nullptr);
    bool haveDeckLink() const;

    // xxx { mode, width, height, fps } ?
    enum InputFormat {
        VideoSDPAL=bmdModePAL,
        VideoSDNTSC=bmdModeNTSC,
        VideoSDPALp=bmdModePALp,
        VideoSDNTSCp=bmdModeNTSCp,

        VideoHD1080p24=bmdModeHD1080p24,
        VideoHD1080p25=bmdModeHD1080p25,
        VideoHD1080p30=bmdModeHD1080p30,
        VideoHD1080p50=bmdModeHD1080p50,
        VideoHD1080p60=bmdModeHD1080p6000,
    };
    Q_ENUM(InputFormat);

    Q_INVOKABLE void setFramebufferSize(QSize size);
    Q_INVOKABLE void setVideoSink(QObject *videosink);

    Q_INVOKABLE bool setInput(uint index);

    Q_INVOKABLE bool setMode(qint32 mode);

    Q_INVOKABLE bool setProfile(uint profile);
    Q_INVOKABLE bool setKeyer(bool enable);    

    QObject *getVideoSink() const;

    QObject *getDecklink() const;
    void setDecklink(QObject *newDecklink);

    bool streaming() const;

signals:
    void haveDeckLinkChanged();
    void videoSinkChanged();
    void devicesChanged();
    void decklinkChanged();
    void frameQueued();
    void inputModeChanged();
    void streamingChanged();
    void restartStream();

public slots:
    bool enableInput();
    bool disableInput();
protected:
    void imageToBuffer(const QImage &frame);
    void newFrame(IDeckLinkVideoInputFrame *frame);
    void modeChanged(qint32 mode, long width, long height, float fps);
    void setStreaming(bool newStreaming);
protected slots:
    void processFrame();

private:
    DeckLink *m_decklink = nullptr;
    DeckLinkInputCallback *m_icb;

    int m_current=-1;
    QSize m_fbsize;
    QVideoSink *m_videosink=nullptr;

    QMutex m_mutex;
    QQueue<IDeckLinkVideoInputFrame *> m_frames;
    uint64_t m_framecounter;

    bool m_streaming=false;
    bool m_audio=false;

    // Active output/input/keyer and frame    
    IDeckLinkInput *m_input=nullptr;
    IDeckLinkKeyer *m_keyer=nullptr;
    IDeckLinkMutableVideoFrame* m_frame=nullptr;

    BMDDisplayMode m_mode=bmdModeHD1080p30; // bmdModeHD1080p6000; //
};

#endif // DECKLINKSOURCE_H
