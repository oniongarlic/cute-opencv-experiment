#ifndef OVVIDEOFILTER_H
#define OVVIDEOFILTER_H

#include <QAbstractVideoFilter>

class OvVideoFilter : public QAbstractVideoFilter
{
public:
    OvVideoFilter(QAbstractVideoFilter *parent=nullptr);

    // QAbstractVideoFilter interface
public:
    QVideoFilterRunnable *createFilterRunnable();
};

#endif // OVVIDEOFILTER_H
