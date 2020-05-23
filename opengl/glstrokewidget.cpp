#include "glstrokewidget.h"

#include "opengl/glstrokerenderer.h"
#include "opengl/glcanvasstroke.h"

#include <QMouseEvent>
#include <QGraphicsProxyWidget>

class MouseStroke : public GLInputStroke
{
private:
    StrokePoint point = { 0, 0, 0, 0, 0 };
    QPointF lpt;

public:
    MouseStroke(GLStrokeRenderer * renderer)
        : GLInputStroke(renderer)
    {
    }

    void setSize(QSize const & size)
    {
        point.x = static_cast<ushort>(size.width());
        point.y = static_cast<ushort>(size.height());
        point.s = 1;
        point.p = 2;
        setMaximun(point);
        point.s = 0;
    }

    void Start(QPoint const & pt)
    {
        point.x = static_cast<ushort>(pt.x());
        point.y = static_cast<ushort>(pt.y());
        point.p = 2;
        lpt = pt;
        addPoint(point);
    }

    void Push(QPoint const & pt)
    {
        point.x = static_cast<ushort>(pt.x());
        point.y = static_cast<ushort>(pt.y());
        auto d = pt - lpt;
        auto dd = d.x() * d.x() + d.y() * d.y();
        point.p = dd > 200 ? 1 : 2;
        lpt = pt;
        addPoint(point);
    }

    void End()
    {
        endStroke();
    }
};

GLStrokeWidget::GLStrokeWidget(QWidget* parent)
    : QOpenGLWidget(parent)
{
    QSurfaceFormat format;
    format.setVersion(3, 3);
    format.setSamples(4);
    //format.setAlphaBufferSize(8);
    setFormat(format);
    setUpdateBehavior(PartialUpdate);
    //setAttribute(Qt::WA_AlwaysStackOnTop);
}

GLStrokeWidget::~GLStrokeWidget()
{
    if (mouseStroke_) {
        mouseStroke_ = nullptr;
    }
}

void GLStrokeWidget::initializeGL()
{
    renderer_ = new GLStrokeRenderer([this]() {
        update();
        QGraphicsProxyWidget * proxy = graphicsProxyWidget();
        if (proxy)
            proxy->update();
    });
    renderer_->InitDraw();
    mouseStroke_ = new MouseStroke(renderer_);
    renderer_->CreateCanvas(mouseStroke_, QRectF(-1.0, -1.0, 2.0, 2.0), false);
}

void GLStrokeWidget::resizeGL(int w, int h)
{
    renderer_->OnSizeChanged(w, h);
    mouseStroke_->setSize({w, h});
}

void GLStrokeWidget::paintGL()
{
    renderer_->Draw();
}

void GLStrokeWidget::mousePressEvent(QMouseEvent *event)
{
    mouseStroke_->Start(event->pos());
}

void GLStrokeWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (mouseStroke_->finished()) {
        QOpenGLWidget::mouseMoveEvent(event);
        return;
    }
    mouseStroke_->Push(event->pos());
}

void GLStrokeWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (mouseStroke_->finished()) {
        QOpenGLWidget::mouseMoveEvent(event);
        return;
    }
    mouseStroke_->End();
}


