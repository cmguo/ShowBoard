#ifndef GLSTROKEWIDGET_H
#define GLSTROKEWIDGET_H

#include "ShowBoard_global.h"

#include <QOpenGLWidget>

class QMouseEvent;
class GLStrokeRenderer;
class MouseStroke;

class SHOWBOARD_EXPORT GLStrokeWidget : public QOpenGLWidget
{
public:
    GLStrokeWidget(QWidget* parent = nullptr);

    virtual ~GLStrokeWidget() override;

private:
    virtual void initializeGL() override;

    virtual void resizeGL(int w, int h) override;

    virtual void paintGL() override;

private:
    virtual void mousePressEvent(QMouseEvent *event) override;

    virtual void mouseMoveEvent(QMouseEvent *event) override;

    virtual void mouseReleaseEvent(QMouseEvent *event) override;

public:
    GLStrokeRenderer * renderer_;
    MouseStroke * mouseStroke_;
};

#endif // GLSTROKEWIDGET_H
