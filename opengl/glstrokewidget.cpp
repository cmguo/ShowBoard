#include "glstrokewidget.h"

#include "opengl/glstrokerenderer.h"
#include "opengl/gldynamicstroke.h"

#include <QMouseEvent>
#include <QGraphicsProxyWidget>

class MouseStroke : public GLDynamicStroke
{
private:
    float point[3] = { 0.0f, 0.0f, 0.0f };
    QPointF lpt;

public:
    MouseStroke(GLStrokeRenderer * renderer)
        : GLDynamicStroke(renderer)
    {
    }

    void Start(QSizeF glc, QPointF pt)
    {
        point[0] = static_cast<float>(pt.x() / glc.width());
        point[1] = static_cast<float>(1 - pt.y() / glc.height());
        point[2] = 2.0f;
        lpt = pt;
        addPoint(point);
    }

    void Push(QSizeF glc, QPointF pt)
    {
        point[0] = static_cast<float>(pt.x() / glc.width());
        point[1] = static_cast<float>(1 - pt.y() / glc.height());
        auto d = pt - lpt;
        double dd = d.x() * d.x() + d.y() * d.y();
        point[2] = dd > 200.0 ? 1.0f : 2.0f;
        lpt = pt;
        addPoint(point);
    }

    void End()
    {
        addPoint(nullptr);
    }
};

GLStrokeWidget::GLStrokeWidget(QWidget* parent)
    : QOpenGLWidget(parent)
{
    QSurfaceFormat format;
    format.setVersion(3, 3);
    format.setSamples(4);
    format.setAlphaBufferSize(8);
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
}

void GLStrokeWidget::paintGL()
{
    renderer_->Draw();
}

void GLStrokeWidget::mousePressEvent(QMouseEvent *event)
{
    QPointF pt = event->pos();
    mouseStroke_->Start(size(), pt);
}

void GLStrokeWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (mouseStroke_->finished()) {
        QOpenGLWidget::mouseMoveEvent(event);
        return;
    }
    QPointF pt = event->pos();
    mouseStroke_->Push(size(), pt);
}

void GLStrokeWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (mouseStroke_->finished()) {
        QOpenGLWidget::mouseMoveEvent(event);
        return;
    }
    mouseStroke_->End();
}


