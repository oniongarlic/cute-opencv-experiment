#include "decklinksink.h"

#include <QUrl>
#include <QDebug>

Decklinksink::Decklinksink(QObject *parent)
    : QObject{parent}
{
    IDeckLinkIterator* deckLinkIterator	= nullptr;
    IDeckLink* deckLink=nullptr;
    HRESULT result;

    deckLinkIterator=CreateDeckLinkIteratorInstance();
    if (deckLinkIterator!=nullptr) {
        qDebug() << "Found DeckLink support";
        m_haveDeckLink=true;
    } else {
        qDebug() << "No DeckLink support installed";
        return;
    }

    m_fbsize.setWidth(1920);
    m_fbsize.setHeight(1080);

    /* Look for devices */
    while ((deckLinkIterator->Next(&deckLink))==S_OK) {
        const char *model, *name, *handle;
        IDeckLinkProfileAttributes* deckLinkAttributes = NULL;
        int64_t value;

        deckLink->GetModelName(&model);
        deckLink->GetDisplayName(&name);

        QVariantMap dev;
        dev["modelName"]=QVariant(model);
        dev["displayName"]=QVariant(name);

        result = deckLink->QueryInterface(IID_IDeckLinkProfileAttributes, (void**)&deckLinkAttributes);
        if (result != S_OK) {
            qWarning("Failed to get decklink device attributes");
            continue;
        }

        result = deckLinkAttributes->GetInt(BMDDeckLinkDeviceInterface, &value);
        if (result != S_OK) {
            qWarning("Failed to get decklink device interface attribute");
            goto out;
        }

        switch(value) {
        case bmdDeviceInterfacePCI:
            dev["interface"]="PCI";
            break;
        case bmdDeviceInterfaceUSB:
            dev["interface"]="USB";
            break;
        case bmdDeviceInterfaceThunderbolt:
            dev["interface"]="Thunderbolt";
            break;
        default:
            qDebug() << "Unknown interface?";
        }

        if (deckLinkAttributes->GetInt(BMDDeckLinkVideoIOSupport, &value) != S_OK) {
            qDebug("No output support, skipping");
            goto out;
        }

        if ((value & bmdDeviceSupportsPlayback) != 0) {
            IDeckLinkDisplayModeIterator *dmi;
            IDeckLinkDisplayMode *dm;
            QVariantList modes;

            result = deckLink->QueryInterface(IID_IDeckLinkOutput, (void**)&m_output);
            m_output->GetDisplayModeIterator(&dmi);
            while ((dmi->Next(&dm))==S_OK) {
                const char *mname;
                int w,h,m;

                QVariantMap mode;

                dm->GetName(&mname);
                h=dm->GetHeight();
                w=dm->GetWidth();
                m=dm->GetDisplayMode();

                // qDebug() << m << mname << w << h;

                mode["name"]=QVariant(mname);
                mode["width"]=w;
                mode["height"]=h;
                mode["mode"]=m;
                modes.append(mode);
            }
            m_outputs.append(m_output);
            dev["playback"]=true;
            dev["modes"]=modes;
        } else {
            qDebug("No playback support, skipping");
            goto out;
        }

        result = deckLinkAttributes->GetInt(BMDDeckLinkNumberOfSubDevices, &value);
        if (result != S_OK) {
            qWarning("Failed to get decklink device interface attribute");
            goto out;
        } else {
            dev["subdevices"]=(qint64)value;
        }

        result = deckLinkAttributes->GetInt(BMDDeckLinkSubDeviceIndex, &value);
        if (result != S_OK) {
            qWarning("Failed to get decklink device interface attribute");
            goto out;
        } else {
            dev["subindex"]=(qint64)value;
        }

        result = deckLinkAttributes->GetInt(BMDDeckLinkVideoOutputConnections, &value);
        if (result != S_OK) {
            qWarning("Failed to get decklink device interface attribute");
            goto out;
        } else {
            dev["outputs"]=(qint64)value;
        }

        result = deckLinkAttributes->GetInt(BMDDeckLinkVideoInputConnections, &value);
        if (result != S_OK) {
            qWarning("Failed to get decklink device interface attribute");
            goto out;
        } else {
            dev["inputs"]=(qint64)value;
        }

        //qDebug() << "Device" << dev;

        m_devices++;
        m_deviceList.append(dev);

    out:;

        deckLinkAttributes->Release();
        deckLink->Release();
    }

    qDebug() << "Found devices" << m_devices << m_deviceList;

    if (m_default>-1)
        setOutput(m_default);

    emit haveDeckLinkChanged();
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
}

bool Decklinksink::setOutput(uint index)
{
    HRESULT result;

    if (index>m_outputs.count())
        return false;
    m_output=m_outputs.at(index);

    if (m_output) {
        result = m_output->CreateVideoFrame(m_fbsize.width(), m_fbsize.height(), m_fbsize.width()*4, bmdFormat8BitBGRA, bmdFrameFlagDefault, &m_frame);
        if (result!=S_OK) {
            qWarning("Failed to create video frame");
            return false;
        }
    } else {
        qWarning("!!! No output set ?");
        return false;
    }

    return true;
}

bool Decklinksink::setMode(qint32 mode)
{
    m_mode=mode;

    return true;
}

void Decklinksink::displayFrame(const QVideoFrame &frame)
{
    if (!m_haveDeckLink)
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

    if (!m_haveDeckLink)
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
    if (!m_haveDeckLink)
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
        qWarning("Failed to enable output");
}

bool Decklinksink::haveDeckLink() const
{
    return m_haveDeckLink;
}

void Decklinksink::setFramebufferSize(QSize size)
{
    m_fbsize=size;
}

int Decklinksink::devices() const
{
    return m_devices;
}
