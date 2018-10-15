#include "ovvideofilter.h"

#include "ovvideofilterrunnable.h"

OvVideoFilter::OvVideoFilter(QAbstractVideoFilter *parent) :
    QAbstractVideoFilter(parent)
{

}

QVideoFilterRunnable *OvVideoFilter::createFilterRunnable()
{
    return new OvVideoFilterRunnable(this);
}
