#include "ocvobjectcolordetector.h"

#include <QtDebug>
#include <QFile>
#include <QJsonObject>
#include <QColor>
#include <QVariantMap>

using namespace cv;

void cv::error(int _code, const std::string& _err, const char* _func, const char* _file, int _line) {
    qDebug() << _code << _func << _file << _line;
    qFatal("%s", _err.c_str());
}

OCVObjectColorDetector::OCVObjectColorDetector(QObject *parent) :
    QObject(parent),
    m_tolerance(20),
    m_distance(9999),
    m_hysterecis(5),
    m_colorIndex(-1),
    m_bs(33)
{
    QFile cf(":colors.json");
    if (!cf.open(QIODevice::ReadOnly)) {
        qWarning("Failed to open color definition file");
        qDebug() << cf.errorString();
        return;
    }

    QByteArray data=cf.readAll();
    QJsonDocument jd=QJsonDocument::fromJson(data);
    QVariantMap cm=jd.object().toVariantMap();

    QMapIterator<QString, QVariant> i(cm);
    while (i.hasNext()) {
        i.next();
        QVariantMap cmap=i.value().toMap();
        QColor c(i.key());
        qDebug() << i.key() << cmap << c;

        Mat lab;
        Mat bgr(1, 1, CV_8UC3, Scalar(c.blue(),c.green(),c.red()));
        cvtColor(bgr, lab, CV_BGR2Lab);

        Vec3b ci = lab.at<Vec3b>(0,0);
        double il = (double)ci.val[0];
        double ia = (double)ci.val[1];
        double ib = (double)ci.val[2];

        qDebug() << i.key() << il << ia << ib;

        m_colors.append(Scalar(il,ia,ib));
        m_colorgroups.append(cmap["group"].toString());
        m_colornames.append(cmap["name"].toString());
        m_rgbcolors.append(i.key());
    }
}

QString OCVObjectColorDetector::getColorGroup() const
{
    if (isValid())
        return m_colorgroups.at(m_colorIndex);
    else
        return "";
}

QString OCVObjectColorDetector::getColorRGB() const
{
    if (isValid())
        return m_rgbcolors.at(m_colorIndex);
    else
        return "";
}

QString OCVObjectColorDetector::getColorName() const
{
    if (isValid())
        return m_colornames.at(m_colorIndex);
    else
        return "";
}

double OCVObjectColorDetector::deltaE(const Scalar &a, const Scalar &b) const
{
    return sqrt(pow(b[0]-a[0], 2)+pow(b[1]-a[1], 2)+pow(b[2]-a[2], 2));
}

bool OCVObjectColorDetector::findClosestMatch(const Scalar &lab, const double tolerance, int &cindex, double &distance) const
{
    bool r=false;
    int ci;
    double dist=999;

    for (int i=0;i<m_colors.size();i++) {
        double de=deltaE(m_colors[i], lab);

        // Skip if distance is very large
        if (de>tolerance)
            continue;

        if (de<dist) {
            ci=i;
            dist=de;
            r=true;
        }
    }
    cindex=ci;
    distance=dist;

    return r;
}

void OCVObjectColorDetector::calculateRoi(Mat &frame, Rect &roi, int ox, int oy)
{    
    int wb=frame.size().width/m_bs;
    int hb=frame.size().height/m_bs;

    roi.x = (m_bs/2*wb)+(ox*wb);
    roi.y = (m_bs/2*hb)+(oy*hb);
    roi.width = wb;
    roi.height = hb;
}

Mat OCVObjectColorDetector::equalizeIntensity(const Mat& inputImage)
{
    Mat ycrcb, result;
    std::vector<Mat> channels;

    cvtColor(inputImage, ycrcb, CV_BGR2YCrCb);
    split(ycrcb, channels);
    equalizeHist(channels[0], channels[0]);
    merge(channels, ycrcb);
    cvtColor(ycrcb, result, CV_YCrCb2BGR);

    return result;
}

bool OCVObjectColorDetector::processImageFile(QString path)
{
    QImage i;

    i.load(path);
    if (i.isNull()) {
        qWarning() << "Failed to load image file " << path;
        return false;
    }

    return processFrame(i);
}

bool OCVObjectColorDetector::processFrame(QImage &frame)
{
    Mat cvframe;

    switch(frame.format()) {
    case QImage::Format_Invalid:
    {
        return false;
    }
    case QImage::Format_RGB32:
    {
        Mat view(frame.height(), frame.width(), CV_8UC4,(void *)frame.constBits(), frame.bytesPerLine());
        view.copyTo(cvframe);
        break;
    }
    case QImage::Format_RGB888:
    {
        Mat view(frame.height(), frame.width(), CV_8UC3,(void *)frame.constBits(), frame.bytesPerLine());
        cvtColor(view, cvframe, COLOR_RGB2BGR);
        break;
    }
    default:
    {
        QImage conv = frame.convertToFormat(QImage::Format_ARGB32);
        Mat view(conv.height(),conv.width(),CV_8UC4,(void *)conv.constBits(),conv.bytesPerLine());
        view.copyTo(cvframe);
        break;
    }
    }

    return processFrame(cvframe);
}

bool OCVObjectColorDetector::processFrame(Mat &frame)
{
    Mat center, rl, rr, ru, rd, n, hls;
    Rect roi, roiL, roiR, roiU, roiD;
    int ci;
    bool r=false;
    double dist=999.0;
    Size blr(4,4);

    calculateRoi(frame, roi, 0, 0);
    calculateRoi(frame, roiL, -2, 0);
    calculateRoi(frame, roiR, 2, 0);
    calculateRoi(frame, roiU, 0, -2);
    calculateRoi(frame, roiD, 0, 2);

    center=frame(roi);
    rl=frame(roiL);
    rr=frame(roiR);
    rd=frame(roiD);
    ru=frame(roiU);

    blur(center, center, blr);
    blur(rl, rl, blr);
    blur(rr, rr, blr);
    blur(rd, rd, blr);
    blur(ru, ru, blr);

    // Average, double weight center of image
    Scalar tmp=mean(center)+mean(center)+mean(rr)+mean(rl)+mean(ru)+mean(rd);
    Scalar bgrm(tmp[0]/6, tmp[1]/6, tmp[2]/6);

    // Convert to Lab color for easy comparission
    Mat ctmp(Size(1,1), CV_8UC3, bgrm);
    Mat ltmp;
    cvtColor(ctmp, ltmp, CV_BGR2Lab);
    Scalar labm=mean(ltmp);

    double dp=m_tolerance;

    if (findClosestMatch(labm, m_tolerance, ci, dist)) {
        // Are we very close to previous color ? If so don't switch over to fast        
        if (isValid())
            dp=deltaE(lab, labm);

        if (isValid() && dp<m_hysterecis) {
            r=true;
        } else {
            lab=labm;
            bgr=bgrm;
            cvtColor(ctmp, hls, CV_BGR2HLS);
            hls=mean(ltmp);
            m_distance=dist;
            m_colorIndex=ci;            
        }
    } else {
        m_distance=9999;       
    }

    qDebug() << isValid() << m_colorIndex << getColorGroup() << getColorName();
    if (isValid()) {
        emit colorFound();
    } else {
        emit colorNotFound();
    }

    return isValid();
}

bool OCVObjectColorDetector::isValid() const
{
    return m_distance>999 ? false : true;
}
