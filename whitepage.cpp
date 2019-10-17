#include "whitepage.h"
#include "resourceview.h"

WhitePage::WhitePage(QObject *parent)
    : QObject(parent)
{
}

void WhitePage::addResource(ResourceView * res)
{
    resources_.append(res);
    res->setParent(this);
}

void WhitePage::removeResource(ResourceView * res)
{
    resources_.removeOne(res);
    delete res;
}
