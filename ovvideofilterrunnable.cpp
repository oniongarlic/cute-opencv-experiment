#include "ovvideofilter.h"

#include "ovvideofilterrunnable.h"

#include <QDebug>
#include <opencv2/imgproc/types_c.h>

// #define DEBUG_FRAMES 1
// #define DEBUG_CVW 1

OvVideoFilterRunnable::OvVideoFilterRunnable(OvVideoFilter *parent) :
    m_parent(parent),
    m_ci(-1)
{
#ifdef DEBUG_CVW
    cv::namedWindow("debug");
#endif    
}

QVideoFrame OvVideoFilterRunnable::run(QVideoFrame *input, const QVideoSurfaceFormat &surfaceFormat, QVideoFilterRunnable::RunFlags flags)
{    
    Q_UNUSED(flags)

    if (!input->isValid()) {
        qWarning("Frame is not valid.");
        return *input;
    }

    if (!input->map(QAbstractVideoBuffer::ReadOnly)) {
        qWarning("Failed to map video frame for reading.");
        return *input;
    }

#if 0
    if (surfaceFormat.handleType()!=QAbstractVideoBuffer::NoHandle) {
        qWarning("Surface format not supported.");
        return *input;
    }
#endif

#ifdef DEBUG_FRAMES
    qDebug() << "Frame format: " << input->pixelFormat() << surfaceFormat.frameRate();
#endif

#if 0
    QImage image(input->bits(),input->width(), input->height(), QVideoFrame::imageFormatFromPixelFormat(input->pixelFormat()));
    image = image.convertToFormat(QImage::Format_RGB888);
    cv::Mat mat(image.height(), image.width(), CV_8UC3, image.bits(), image.bytesPerLine());
    m_frame=cv::Mat(image.height(), image.width(), CV_8UC3);
    cvtColor(mat, m_frame, CV_RGB2BGR);
#else
    if (frameToImage(*input)) {
#ifdef DEBUG_CVW
        cv::imshow("debug", m_frame);
#endif
        m_cd.processOpenCVFrame(m_frame);

        if (m_cd.isValid()) {
            int i=m_cd.colorIndex();
            if (i!=m_ci) {
                emit m_parent->colorFound(m_cd.getColorGroup(), m_cd.getColorName());
                m_ci=i;
            }
        }
    }
#endif
    input->unmap();

    return *input;
}

/**
 * Why these aren't public in Qt I can't understand...
 * Ripped from Qt sources:
 * qtmultimedia/src/multimedia/video/qvideoframeconversionhelper.cpp
 *
 * ** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
 */

#define CLAMP(n) (n > 255 ? 255 : (n < 0 ? 0 : n))

#define EXPAND_UV(u, v) \
    int uu = u - 128; \
    int vv = v - 128; \
    int rv = 409 * vv + 128; \
    int guv = 100 * uu + 208 * vv + 128; \
    int bu = 516 * uu + 128; \


#define FETCH_INFO_BIPLANAR(frame) \
    const uchar *plane1 = frame.bits(0); \
    const uchar *plane2 = frame.bits(1); \
    int plane1Stride = frame.bytesPerLine(0); \
    int plane2Stride = frame.bytesPerLine(1); \
    int width = frame.width(); \
    int height = frame.height();

#define FETCH_INFO_TRIPLANAR(frame) \
    const uchar *plane1 = frame.bits(0); \
    const uchar *plane2 = frame.bits(1); \
    const uchar *plane3 = frame.bits(2); \
    int plane1Stride = frame.bytesPerLine(0); \
    int plane2Stride = frame.bytesPerLine(1); \
    int plane3Stride = frame.bytesPerLine(2); \
    int width = frame.width(); \
    int height = frame.height(); \

static inline quint32 qYUVToARGB32(int y, int rv, int guv, int bu, int a = 0xff)
{
    int yy = (y - 16) * 298;
    return (a << 24)
            | CLAMP((yy + rv) >> 8) << 16
            | CLAMP((yy - guv) >> 8) << 8
            | CLAMP((yy + bu) >> 8);
}

static inline void planarYUV420_to_ARGB32(const uchar *y, int yStride,
                                          const uchar *u, int uStride,
                                          const uchar *v, int vStride,
                                          int uvPixelStride,
                                          quint32 *rgb,
                                          int width, int height)
{
    quint32 *rgb0 = rgb;
    quint32 *rgb1 = rgb + width;
    for (int j = 0; j < height; j += 2) {
        const uchar *lineY0 = y;
        const uchar *lineY1 = y + yStride;
        const uchar *lineU = u;
        const uchar *lineV = v;
        for (int i = 0; i < width; i += 2) {
            EXPAND_UV(*lineU, *lineV);
            lineU += uvPixelStride;
            lineV += uvPixelStride;
            *rgb0++ = qYUVToARGB32(*lineY0++, rv, guv, bu);
            *rgb0++ = qYUVToARGB32(*lineY0++, rv, guv, bu);
            *rgb1++ = qYUVToARGB32(*lineY1++, rv, guv, bu);
            *rgb1++ = qYUVToARGB32(*lineY1++, rv, guv, bu);
        }
        y += yStride << 1; // stride * 2
        u += uStride;
        v += vStride;
        rgb0 += width;
        rgb1 += width;
    }
}

void QT_FASTCALL qt_convert_YUV420P_to_ARGB32(const QVideoFrame &frame, uchar *output)
{
    FETCH_INFO_TRIPLANAR(frame)
    planarYUV420_to_ARGB32(plane1, plane1Stride,
                           plane2, plane2Stride,
                           plane3, plane3Stride,
                           1,
                           reinterpret_cast<quint32*>(output),
                           width, height);
}
void QT_FASTCALL qt_convert_YV12_to_ARGB32(const QVideoFrame &frame, uchar *output)
{
    FETCH_INFO_TRIPLANAR(frame)
    planarYUV420_to_ARGB32(plane1, plane1Stride,
                           plane3, plane3Stride,
                           plane2, plane2Stride,
                           1,
                           reinterpret_cast<quint32*>(output),
                           width, height);
}

void QT_FASTCALL qt_convert_NV12_to_ARGB32(const QVideoFrame &frame, uchar *output)
{
    FETCH_INFO_BIPLANAR(frame)
    planarYUV420_to_ARGB32(plane1, plane1Stride,
                           plane2, plane2Stride,
                           plane2 + 1, plane2Stride,
                           2,
                           reinterpret_cast<quint32*>(output),
                           width, height);
}
void QT_FASTCALL qt_convert_NV21_to_ARGB32(const QVideoFrame &frame, uchar *output)
{
    FETCH_INFO_BIPLANAR(frame)
    planarYUV420_to_ARGB32(plane1, plane1Stride,
                           plane2 + 1, plane2Stride,
                           plane2, plane2Stride,
                           2,
                           reinterpret_cast<quint32*>(output),
                           width, height);
}

bool OvVideoFilterRunnable::frameToImage(const QVideoFrame &input)
{
    QVideoFrame::PixelFormat m_format=input.pixelFormat();

    int w=input.width();
    int h=input.height();

    switch (m_format) {
    case QVideoFrame::Format_UYVY:
        qWarning() << "Unhandled VideoFrame format";
        break;
    case QVideoFrame::Format_RGB32:
        m_frame=cv::Mat(h, w, CV_8UC3, (void*) input.bits());
        return true;
    case QVideoFrame::Format_ARGB32:
    case QVideoFrame::Format_ARGB32_Premultiplied:
        m_frame=cv::Mat(h, w, CV_8UC4, (void*) input.bits());
        return true;
    case QVideoFrame::Format_RGB565:
    case QVideoFrame::Format_RGB555: {
        m_frame=cv::Mat(h, w, CV_8UC3, (void*) input.bits());
        //cvtColor(m_frame, m_frame, CV_RGB2BGR555, 3);
        return true;
    }
    case QVideoFrame::Format_BGR32: {
        m_frame=cv::Mat(h, w, CV_8UC3, (void*) input.bits(), input.bytesPerLine());
        return true;
    }
    case QVideoFrame::Format_NV12: {
        m_result = QImage(input.width(), input.height(), QImage::Format_ARGB32);
        qt_convert_NV12_to_ARGB32(input, m_result.bits());
        m_frame=cv::Mat(h, w, CV_8UC4, (void*) m_result.bits());
        return true;
    }
    case QVideoFrame::Format_NV21: {
        m_result = QImage(input.width(), input.height(), QImage::Format_ARGB32);
        qt_convert_NV21_to_ARGB32(input, m_result.bits());
        m_frame=cv::Mat(h, w, CV_8UC4, (void*) m_result.bits());
        return true;
    }
    case QVideoFrame::Format_YUV420P: {
#if 0
        m_result = QImage(input.width(), input.height(), QImage::Format_ARGB32);
        qt_convert_YUV420P_to_ARGB32(input, m_result.bits());
        m_frame=cv::Mat(h, w, CV_8UC4, (void*) m_result.bits());
        cvtColor(m_frame, m_frame, CV_RGBA2RGB, 3);
#else
        cv::Mat mYUV(h + h/2, w, CV_8UC1, (void*) input.bits());
        cvtColor(mYUV, m_frame, CV_YUV2RGB_YV12, 3);
#endif
        return true;
    }
    default:;
        qWarning() << "Unhandled VideoFrame format";
    }
    return false;
}
