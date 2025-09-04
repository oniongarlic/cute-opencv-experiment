#include "decklinksource.h"

#include <QUrl>
#include <QDebug>

class DeckLinkInputCallback: public IDeckLinkInputCallback
{
public:
    DeckLinkInputCallback(Decklinksource *src) :
        m_src(src)
    {
    }

    HRESULT STDMETHODCALLTYPE VideoInputFormatChanged(BMDVideoInputFormatChangedEvents notificationEvents, IDeckLinkDisplayMode *newDisplayMode, BMDDetectedVideoInputFormatFlags detectedSignalFlags) override {

        qDebug() << "VideoInputFormatChanged" << notificationEvents << detectedSignalFlags;

        qDebug() << "Mode: " << newDisplayMode->GetDisplayMode() << newDisplayMode->GetWidth() << newDisplayMode->GetHeight() << newDisplayMode->GetFlags();

        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE VideoInputFrameArrived(IDeckLinkVideoInputFrame* videoFrame, IDeckLinkAudioInputPacket* audioPacket) override {
        QImage frame;
        long w,h;
        uint f;

        if (!videoFrame || (videoFrame->GetFlags() & bmdFrameHasNoInputSource))
        {
            qDebug("No valid input frame\n");
            return S_OK;
        }

        videoFrame->AddRef();
        h=videoFrame->GetHeight();
        w=videoFrame->GetWidth();
        f=videoFrame->GetPixelFormat();

        qDebug() << "Got frame" << h << w << f;

        m_src->newFrame(videoFrame);

        return S_OK;
    }
    HRESULT	STDMETHODCALLTYPE QueryInterface(REFIID iid, LPVOID *ppv) override { return E_NOINTERFACE; }

    ULONG STDMETHODCALLTYPE AddRef() override { return 1; }
    ULONG STDMETHODCALLTYPE Release() override { return 1; }

private:
    Decklinksource*  m_src;        
};

Decklinksource::Decklinksource(QObject *parent)
    : QObject{parent}
{
    m_fbsize.setWidth(1920);
    m_fbsize.setHeight(1080);
    m_audio=false;
    m_icb=new DeckLinkInputCallback(this);

    QObject::connect(this, &Decklinksource::frameQueued,
                     this, &Decklinksource::processFrame,
                     Qt::QueuedConnection);
}

void Decklinksource::setVideoSink(QObject *videosink)
{

}

QObject *Decklinksource::getVideoSink() const
{
    return m_videosink;
}

bool Decklinksource::setInput(uint index)
{
    HRESULT result;
    DeckLinkDevice *d;

    if (!m_decklink->haveDeckLink()) {
        qWarning("No decklink device found");
        return false;
    }

    if (m_streaming) {
        qWarning("Input device in use, can not change");
        return false;
    }

    d=m_decklink->getDevice(index);

    if (!d) {
        qWarning("Invalid decklink device");
        return false;
    }    

    m_current=index;
    m_input=d->input;
    m_keyer=d->key;

    qDebug() << "Decklink input set to " << m_current << d->name;

    m_input->SetCallback(m_icb);

    return true;
}

bool Decklinksource::setMode(qint32 mode)
{
    m_mode=mode;

    return true;
}

bool Decklinksource::setProfile(uint profile)
{
    HRESULT result;
    IDeckLinkProfileManager *manager = NULL;
    IDeckLinkProfile *lp = NULL;
    BMDProfileID profile_id=0;

    if (!m_decklink->haveDeckLink())
        return false;

    if (m_current<0) {
        qWarning("No device set!");
        return false;
    }

    DeckLinkDevice *d=m_decklink->getDevice(m_current);

    if (d->dev->QueryInterface (IID_IDeckLinkProfileManager, (void **) &manager) != S_OK) {
        qWarning("Current device does not support profiles");
        return false;
    }

    switch (profile) {
    case 0:
        profile_id=0;
        break;
    case 1:
        profile_id=bmdProfileOneSubDeviceFullDuplex;
        break;
    case 2:
        profile_id=bmdProfileOneSubDeviceHalfDuplex;
        break;
    case 3:
        profile_id=bmdProfileTwoSubDevicesFullDuplex;
        break;
    case 4:
        profile_id=bmdProfileTwoSubDevicesHalfDuplex;
        break;
    case 5:
        profile_id=bmdProfileFourSubDevicesHalfDuplex;
        break;
    }

    result=manager->GetProfile(profile_id, &lp);

    if (result==S_OK && profile) {
        result=lp->SetActive();
        lp->Release();
    }

    manager->Release();

    return result==S_OK;
}

bool Decklinksource::setKeyer(bool enable)
{
    HRESULT result;
    if (!m_keyer) {
        qWarning("Keyer not set");
        return false;
    }

    if (enable)
        result=m_keyer->Enable(true);
    else
        result=m_keyer->Disable();

    qDebug() << "Keyer set " << (result==S_OK);

    return result==S_OK;
}

void Decklinksource::imageToBuffer(const QImage &frame)
{
    uint8_t* deckLinkBuffer=nullptr;

    if (m_frame->GetBytes((void**)&deckLinkBuffer) != S_OK) {
        qWarning("Failed to get buffer pointer");
        return;
    }

    for (int i=0;i<frame.height(); i++) {
        memcpy(deckLinkBuffer, frame.constScanLine(i), frame.bytesPerLine());
        deckLinkBuffer += m_frame->GetRowBytes();
    }
}

void Decklinksource::newFrame(IDeckLinkVideoInputFrame *frame)
{
    QMutexLocker locker(&m_mutex);
    m_frames.enqueue(frame);
    emit frameQueued();
}

void Decklinksource::processFrame()
{
    QMutexLocker locker(&m_mutex);
    uint8_t* deckLinkBuffer=nullptr;

    if (m_frames.isEmpty()) {
        qWarning("No frames queueed ?");
        return;
    }

    IDeckLinkVideoInputFrame *frame=m_frames.dequeue();

    locker.unlock();

    m_framecounter++;

    if (frame->GetBytes((void**)&deckLinkBuffer)!= S_OK)
        goto out;

    // XXX do something with the frame, duh

    out:

    frame->Release();
}

bool Decklinksource::enableInput()
{    
    if (m_input==nullptr) {
        qWarning("No input");
        return false;
    }

    if (m_audio) {
        m_input->EnableAudioInput(bmdAudioSampleRate48kHz, bmdAudioSampleType16bitInteger, 2);
    }

    if (m_input->EnableVideoInput(m_mode, bmdFormat8BitBGRA, bmdVideoInputFlagDefault | bmdVideoInputEnableFormatDetection)!=S_OK) {
        qWarning("Failed to enable input");
        return false;
    }
    if (m_input->StartStreams()!=S_OK) {
        qWarning("Failed to start stream input");
        return false;
    }

    m_streaming=true;

    return true;
}

bool Decklinksource::disableInput()
{
    if (m_input==nullptr) {
        qWarning("No input");
        return false;
    }

    if (m_input->StopStreams()!=S_OK) {
        qWarning("Failed to stop stream input");
        return false;
    }

    if (m_audio) {
        m_input->DisableAudioInput();
    }


    if (m_input->DisableVideoInput()!=S_OK) {
        qWarning("Failed to disable input");
        return false;
    }

    m_streaming=false;

    return true;
}

void Decklinksource::setFramebufferSize(QSize size)
{
    m_fbsize=size;
}

QObject *Decklinksource::getDecklink() const
{
    return m_decklink;
}

void Decklinksource::setDecklink(QObject *newDecklink)
{
    if (m_decklink == newDecklink)
        return;

    qDebug() << "Decklink set for sink " << this->objectName();

    m_decklink = qobject_cast<DeckLink*>(newDecklink);
    emit decklinkChanged();
}
