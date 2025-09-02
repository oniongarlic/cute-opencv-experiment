#include "decklinksink.h"

#include <QUrl>
#include <QDebug>

Decklinksink::Decklinksink(QObject *parent)
    : QObject{parent}
{
    m_fbsize.setWidth(1920);
    m_fbsize.setHeight(1080);
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

    return result==S_OK;
}

bool Decklinksink::setKeyer(bool enable)
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

    if (m_frame->GetBytes((void**)&deckLinkBuffer) != S_OK) {
        qWarning("Failed to get buffer pointer");
        return;
    }

    memset(deckLinkBuffer, 0, m_fbsize.width()*m_fbsize.height()*4);

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

    switch (image.metaType().id()) {
    case QMetaType::QUrl: {
        QUrl tmp=image.value<QUrl>();

        if (tmp.scheme()=="" || tmp.scheme()=="file") { // File or absolute path
            QImage tmpi;
            tmpi.load(tmp.path());
        } else {
            qDebug() << "Unhandled QUrl scheme" << tmp.scheme();
        }
        displayImage(tmp);
    }
    break;
    case QMetaType::QImage: {
        const QImage tmp=image.value<QImage>();
        displayImage(tmp.rgbSwapped());
    }
    break;
    default:
        return;
    }
}

void Decklinksink::enableOutput()
{    
    if (m_output==nullptr) {
        qWarning("No output");
        return;
    }    

    if (m_output->EnableVideoOutput(m_mode, bmdVideoOutputFlagDefault)!=S_OK)
        qWarning("Failed to enable output");
}

void Decklinksink::disableOutput()
{
    if (m_output==nullptr) {
        qWarning("No output");
        return;
    }

    if (m_output->DisableVideoOutput()!=S_OK)
        qWarning("Failed to disable output");
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
