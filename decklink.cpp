#include "decklink.h"

#include <QUrl>
#include <QTimer>
#include <QDebug>

DeckLink::DeckLink(QObject *parent)
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

    /* Look for devices */
    while ((deckLinkIterator->Next(&deckLink))==S_OK) {
        const char *model, *name;
        IDeckLinkProfileAttributes* deckLinkAttributes = NULL;
        int64_t value;

        DeckLinkDevice *dld=new DeckLinkDevice;
        IDeckLinkInput *input=nullptr;
        IDeckLinkOutput *output=nullptr;

        deckLink->GetModelName(&model);
        deckLink->GetDisplayName(&name);

        dld->dev=deckLink;
        dld->name=name;
        dld->model=model;
        dld->valid=true;

        dld->input=nullptr;
        dld->output=nullptr;
        dld->key=nullptr;

        QVariantMap dev;
        dev["modelName"]=QVariant(model);
        dev["displayName"]=QVariant(name);

        qDebug() << "*** Device: "<< dld->name;

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

        if (deckLinkAttributes->GetInt(BMDDeckLinkPersistentID, &value) == S_OK) {
            dev["PersistentID"]=(qint64)(value);
        } else {
            dev["PersistentID"]=0;
        }

        if (deckLinkAttributes->GetInt(BMDDeckLinkTopologicalID, &value) == S_OK) {
            dev["TopologicalID"]=(qint64)(value);
        } else {
            dev["TopologicalID"]=0;
        }

        if ((value & bmdDeviceSupportsPlayback) != 0) {
            IDeckLinkDisplayModeIterator *dmi;
            IDeckLinkDisplayMode *dm;
            IDeckLinkKeyer *k;
            QVariantList modes;

            result = deckLink->QueryInterface(IID_IDeckLinkOutput, (void**)&output);
            output->GetDisplayModeIterator(&dmi);
            while ((dmi->Next(&dm))==S_OK) {
                const char *mname;
                qint64 w,h;
                uint m;

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
            dmi->Release();

            result = deckLink->QueryInterface (IID_IDeckLinkKeyer, (void **) &k);
            if (result != S_OK) {
                qDebug("No keyer for output");
                dev["keyer"]=false;
                dld->key=nullptr;
            } else {
                bool ki, ke;
                qDebug("Keyer supported");
                dev["keyer"]=true;
                dld->key=k;

                result = deckLinkAttributes->GetFlag(BMDDeckLinkSupportsInternalKeying, &ki);
                result = deckLinkAttributes->GetFlag(BMDDeckLinkSupportsExternalKeying, &ke);

                qDebug() << "Int, Ext" << ki << ke;
            }

            IDeckLinkProfileManager *manager = NULL;
            if (deckLink->QueryInterface (IID_IDeckLinkProfileManager, (void **) &manager) == S_OK) {
                qDebug("Has duplex/profiles mode");
                dev["profiles"]=true;

                int64_t current;
                result=deckLinkAttributes->GetInt(BMDDeckLinkProfileID, &current);

                qDebug() << "Current profile ID" << current;

                dev["profile"]=QVariant((qint64)current);

                IDeckLinkProfileIterator *pi;
                IDeckLinkProfile *p;
                manager->GetProfiles(&pi);

                while ((pi->Next(&p))==S_OK) {
                    bool pactive;
                    IDeckLinkProfileAttributes*	pa;
                    int64_t pid=0;

                    result=p->QueryInterface(IID_IDeckLinkProfileAttributes, (void**)&pa);
                    if (result==S_OK) {
                        result=pa->GetInt(BMDDeckLinkProfileID, &pid);
                    }
                    p->IsActive(&pactive);
                    qDebug() << "*** Profile: " << pid << pactive;
                }

#if 0
                IDeckLinkProfile *profile = NULL;
                BMDProfileID bmd_profile_id = 0;
                result = manager->GetProfile(bmd_profile_id, &profile);

                if (result==S_OK && profile) {
                    result=profile->SetActive();
                    profile->Release();
                }
#endif

                manager->Release();
            } else {
                dev["profiles"]=false;
            }

            dev["playback"]=true;
            dev["outputModes"]=modes;

            dld->output=output;
        }        

        if ((value & bmdDeviceSupportsCapture) != 0) {
            QVariantList modes;

            result = deckLink->QueryInterface(IID_IDeckLinkInput, (void**)&input);
            if (result != S_OK) {

            } else {
                IDeckLinkDisplayModeIterator *dmi;
                IDeckLinkDisplayMode *dm;
                dld->input=input;

                output->GetDisplayModeIterator(&dmi);
                while ((dmi->Next(&dm))==S_OK) {
                    const char *mname;
                    qint64 w,h;
                    uint m;
                    BMDTimeValue tv;
                    BMDTimeScale ts;

                    QVariantMap mode;

                    dm->GetName(&mname);
                    h=dm->GetHeight();
                    w=dm->GetWidth();
                    m=dm->GetDisplayMode();
                    dm->GetFrameRate(&tv, &ts);

                    // qDebug() << "INPUT MODE: " << m << mname << w << h << tv << ts;

                    mode["name"]=QVariant(mname);
                    mode["width"]=w;
                    mode["height"]=h;
                    mode["mode"]=m;
                    modes.append(mode);
                }
                dmi->Release();

                dev["record"]=true;
                dev["inputModes"]=modes;
            }
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
        dld->properties=dev;
        m_devs.append(dld);

    out:;

        deckLinkAttributes->Release();
        // deckLink->Release();
    }

    qDebug() << "Found devices" << m_devices;

    QTimer::singleShot(0, this, [&]{
        emit haveDeckLinkChanged();
        emit devicesChanged();
    });

}

bool DeckLink::haveDeckLink() const
{
    return m_haveDeckLink;
}

int DeckLink::devices() const
{
    return m_devices;
}

QVariantMap DeckLink::getDeviceProperties(int index)
{
    if (index>m_devs.count()-1 || index<0) {
        qWarning("Requested device out of range");
        return QVariantMap();
    }
    auto d=m_devs.at(index);

    return d->properties;
}

DeckLinkDevice *DeckLink::getDevice(int index)
{    
    if (index>m_devs.count()-1 || index<0) {
        qWarning("Requested device out of range");
        return nullptr;
    }

    return m_devs.at(index);
}
