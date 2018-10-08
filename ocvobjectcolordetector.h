#ifndef OCVOBJECTCOLORDETECTOR_H
#define OCVOBJECTCOLORDETECTOR_H

#include <QObject>
#include <QImage>
#include <QJsonDocument>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgproc/types_c.h>

#include "cuteopencvbase.h"

class OCVObjectColorDetector : public CuteOpenCVBase
{
    Q_OBJECT
    Q_PROPERTY(double tolerance READ tolerance WRITE setTolerance NOTIFY toleranceChanged)    

public:
    explicit OCVObjectColorDetector(QObject *parent = nullptr);

    bool processOpenCVFrame(cv::Mat &frame);

    double tolerance() const
    {
        return m_tolerance;
    }

    Q_INVOKABLE void setROI(double x, double y);

    Q_INVOKABLE bool isValid() const;
    Q_INVOKABLE QString getColorGroup() const;
    Q_INVOKABLE QString getColorRGB() const;
    Q_INVOKABLE QString getColorName() const;
    Q_INVOKABLE bool processImageFile(QString path);

    int colorIndex() const
    {
        return m_colorIndex;
    }

private:
    cv::Mat equalizeIntensity(const cv::Mat &inputImage);
    void calculateRoi(cv::Mat &frame, cv::Rect &roi, int ox, int oy);
    double deltaE76(const cv::Scalar &a, const cv::Scalar &b) const;
    bool findClosestMatch(const cv::Scalar &lab, const double tolerance, int &cindex, double &distance) const;

signals:
    void toleranceChanged(double tolerance);    
    void colorFound();
    void colorNotFound();

public slots:

    void setTolerance(double tolerance)
    {
        qWarning("Floating point comparison needs context sanity check");
        if (qFuzzyCompare(m_tolerance, tolerance))
            return;

        m_tolerance = tolerance;
        m_hysterecis = m_tolerance/4.0;
        emit toleranceChanged(m_tolerance);
    }

private:
    double m_tolerance;
    double m_distance;
    double m_hysterecis;
    cv::Scalar lab;
    cv::Scalar bgr;
    cv::Scalar hls;
    QVector<cv::Scalar> m_colors;
    QVector<QString> m_rgbcolors;
    QVector<QString> m_colorgroups;
    QVector<QString> m_colornames;
    int m_colorIndex;
    int m_bs;
    int m_br;
    double m_roix;
    double m_roiy;
};

#endif // OCVOBJECTCOLORDETECTOR_H
