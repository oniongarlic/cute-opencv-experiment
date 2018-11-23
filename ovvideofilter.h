#ifndef OVVIDEOFILTER_H
#define OVVIDEOFILTER_H

#include <QAbstractVideoFilter>

class OvVideoFilter : public QAbstractVideoFilter
{
    Q_OBJECT
public:
    OvVideoFilter(QAbstractVideoFilter *parent=nullptr);

    // QAbstractVideoFilter interface
public:
    QVideoFilterRunnable *createFilterRunnable();

signals:
    void colorFound(const QString cgroup, const QString cname);

};

#endif // OVVIDEOFILTER_H
