#include "decklinksink.h"

#include <QUrl>
#include <QDebug>

Decklinksink::Decklinksink(QObject *parent)
    : QObject{parent}
{
    m_fbsize.setWidth(1920);
    m_fbsize.setHeight(1080);

    QAudioFormat af;
    af.setSampleRate(48000);
    af.setChannelCount(2);
    af.setSampleFormat(QAudioFormat::Int16);

    m_audiosink=new QAudioSink(af, this);
}

void Decklinksink::setVideoSink(QObject *videosink)
{
    if (videosink==nullptr)
        return;

    if (m_videosink) {
        disconnect(m_videosink, nullptr, this, nullptr);
        m_videosink=nullptr;
    }
    m_videosink = qobject_cast<QVideoSink*>(videosink);

    connect(m_videosink, &QVideoSink::videoFrameChanged, this, &Decklinksink::displayFrame);

    emit videoSinkChanged();
}

QObject *Decklinksink::getVideoSink() const
{
    return m_videosink;
}

bool Decklinksink::setOutput(uint index)
{
    HRESULT result;
    DeckLinkDevice *d;

    if (!m_decklink->haveDeckLink()) {
        qWarning("No decklink device found");
        return false;
    }

    d=m_decklink->getDevice(index);

    if (!d) {
        qWarning("Invalid decklink device");
        return false;
    }    

    m_current=index;
    m_output=d->output;
    m_keyer=d->key;

    qDebug() << "Decklink output set to " << m_current << d->name;

    result = m_output->CreateVideoFrame(m_fbsize.width(), m_fbsize.height(), m_fbsize.width()*4, bmdFormat8BitBGRA, bmdFrameFlagDefault, &m_frame);
    if (result!=S_OK) {
        qWarning("Failed to create video frame");
        return false;
    }

    return true;
}

qint32 Decklinksink::getMode()
{
    return m_mode;
}

bool Decklinksink::setMode(qint32 mode)
{
    m_mode=mode;

    return true;
}

bool Decklinksink::setProfile(uint profile)
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

    qDebug() << "Profile set " << profile_id << (bool)(result==S_OK);

    return result==S_OK;
}

bool Decklinksink::setKeyer(bool enable, uint8_t level, bool external)
{
    HRESULT result;

    if (!m_decklink->haveDeckLink())
        return false;

    if (!m_keyer) {
        qWarning("Keyer not set");
        return false;
    }

    if (enable) {
        qDebug("*** Enable key");
        result=m_keyer->Enable(external);
        m_keyEnabled=result==S_OK ? true : false;
    } else {
        qDebug("*** Disable key");
        result=m_keyer->Disable();
        m_keyEnabled=result==S_OK ? false : true;
    }

    // Make sure a blending level is set, default seems to be a bit random
    keyerLevel(level);

    qDebug() << "Keyer set to: " << m_keyEnabled;

    return result==S_OK;
}

bool Decklinksink::keyerLevel(uint8_t level)
{
    HRESULT result;

    if (!m_decklink->haveDeckLink())
        return false;

    if (!m_keyer) {
        qWarning("Keyer not set");
        return false;
    }

    result=m_keyer->SetLevel(level);

    return result==S_OK;
}

bool Decklinksink::keyerRampUp(uint32_t frames)
{
    HRESULT result;

    if (!m_decklink->haveDeckLink())
        return false;

    if (!m_keyer) {
        qWarning("Keyer not set");
        return false;
    }

    result=m_keyer->RampUp(frames);

    return result==S_OK;
}

bool Decklinksink::keyerRampDown(uint32_t frames)
{
    HRESULT result;

    if (!m_decklink->haveDeckLink())
        return false;

    if (!m_keyer) {
        qWarning("Keyer not set");
        return false;
    }

    result=m_keyer->RampDown(frames);

    return result==S_OK;
}

void Decklinksink::displayFrame(const QVideoFrame &frame)
{
    if (!m_decklink->haveDeckLink())
        return;

    if (!m_output)
        return;

    QImage img=frame.toImage();
    QImage f;

    switch (img.format()) {
    case QImage::Format_ARGB32:
        f=img;
        break;
    case QImage::Format_RGB32:
        f=img.convertToFormat(QImage::Format_ARGB32);
        break;
    case QImage::Format_RGB888:
        f=img.convertToFormat(QImage::Format_ARGB32);
        break;
    case QImage::Format_Invalid:
        return;
    default: {
        f=img.convertToFormat(QImage::Format_ARGB32);
    }
    }
    displayImage(f);
}

void Decklinksink::displayImage(const QImage &frame)
{
    QImage f;

    if (!m_decklink->haveDeckLink())
        return;

    if (!m_output)
        return;

    if (frame.size()==m_fbsize && frame.format()==QImage::Format_ARGB32) {
        f=frame;
    } else if (frame.size()==m_fbsize) {
        f=frame.convertToFormat(QImage::Format_ARGB32);
    } else {
        f=frame.scaled(m_fbsize.width(), m_fbsize.height(), Qt::KeepAspectRatio, Qt::SmoothTransformation).convertToFormat(QImage::Format_ARGB32);
    }

    imageToBuffer(f);

    m_output->DisplayVideoFrameSync(m_frame);
}

void Decklinksink::clearBuffer()
{
    uint8_t* deckLinkBuffer=nullptr;

    if (!m_decklink->haveDeckLink())
        return;

    if (!m_output)
        return;

    if (m_frame->GetBytes((void**)&deckLinkBuffer) != S_OK) {
        qWarning("Failed to get buffer pointer");
        return;
    }

    memset(deckLinkBuffer, 128, m_fbsize.width()*m_fbsize.height()*4);

    m_output->DisplayVideoFrameSync(m_frame);
}

void Decklinksink::imageToBuffer(const QImage &frame)
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

void Decklinksink::displayImage(const QVariant image)
{
    if (!m_decklink->haveDeckLink())
        return;

    if (!m_output)
        return;

    switch (image.metaType().id()) {
    case QMetaType::QUrl: {
        QUrl tmp=image.value<QUrl>();
        QImage img;

        if (tmp.scheme()=="" || tmp.scheme()=="file") { // File or absolute path            
            img.load(tmp.path());
        } else {
            qDebug() << "Unhandled QUrl scheme" << tmp.scheme();
            return;
        }
        displayImage(img);
    }
    break;
    case QMetaType::QImage: {
        const QImage tmp=image.value<QImage>();
        displayImage(tmp.rgbSwapped());
    }
    break;
    default:
        qWarning() << "Unhandled image source";
        return;
    }
}

bool Decklinksink::enableOutput()
{
    HRESULT result;

    if (m_output==nullptr) {
        qWarning("No output");
        return false;
    }

    result=m_output->EnableVideoOutput(m_mode, bmdVideoOutputFlagDefault);
    switch (result) {
    case S_OK:
        return true;
        break;
    case E_UNEXPECTED:
        qWarning("Error: Unexpected");
        break;
    case E_OUTOFMEMORY:
        qWarning("Error: Out of memory");
        break;
    case E_ACCESSDENIED:
        qWarning("Error: Access denied");
        break;
    case E_INVALIDARG:
        qWarning("Error: Invalid argument");
        break;
    case E_FAIL:
        qWarning("Error: Failed");
        break;
    default:
        qWarning("Error: Other error");
        break;
    }

    qWarning() << "Failed to enable output, mode " << m_mode << HRESULT_CODE(result);

    return false;
}

bool Decklinksink::disableOutput()
{
    HRESULT result;

    if (m_output==nullptr) {
        qWarning("No output");
        return false;
    }

    result=m_output->DisableVideoOutput();

    if (result!=S_OK) {
        qWarning() << "Failed to disable output" << result;
        return false;
    }
    return true;
}

void Decklinksink::setFramebufferSize(QSize size)
{
    m_fbsize=size;
}

QObject *Decklinksink::getDecklink() const
{
    return m_decklink;
}

void Decklinksink::setDecklink(QObject *newDecklink)
{
    if (m_decklink == newDecklink)
        return;

    qDebug() << "Decklink set for sink " << this->objectName();

    m_decklink = qobject_cast<DeckLink*>(newDecklink);
    emit decklinkChanged();
}

bool Decklinksink::keyEnabled() const
{
    return m_keyEnabled;
}

QObject *Decklinksink::getAudioSink() const
{
    return m_audiosink;
}
