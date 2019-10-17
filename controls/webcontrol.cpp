#include "webcontrol.h"
#include "resource.h"
#include "resourceview.h"

#include <QWebEngineView>

WebControl::WebControl(ResourceView * res)
    : WidgetControl(res)
{

}

QWidget * WebControl::createWidget(ResourceView * res)
{
    QWebEngineView * view = new QWebEngineView();
    view->load(res->resource()->url());
    return view;
}
