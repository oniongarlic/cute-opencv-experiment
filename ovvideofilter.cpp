#include "ovvideofilter.h"

#include "ovvideofilterrunnable.h"

OvVideoFilter::OvVideoFilter(QAbstractVideoFilter *parent) :
    QAbstractVideoFilter(parent)
{

}

QVideoFilterRunnable *OvVideoFilter::createFilterRunnable()
{
    QVideoFilterRunnable *rf=new OvVideoFilterRunnable(this);    

    return rf;
}
