#include "strokecontrol.h"

#include "opengl/glstrokewidget.h"

#include "resources/strokes.h"

#include <QGraphicsProxyWidget>

StrokeControl::StrokeControl(ResourceView * res)
    : WidgetControl(res, FullLayout, DefaultFlags)
{
}

QWidget * StrokeControl::createWidget(ResourceView * res)
{
    QWidget * widget = new GLStrokeWidget();
    widget->resize(720, 405);
    //widget->setAttribute(Qt::WA_AlwaysStackOnTop);
    qobject_cast<Strokes *>(res)->load().then([]() {
    });
    return widget;
}
