#ifndef WHITECANVASCONTROL_H
#define WHITECANVASCONTROL_H

#include "core/control.h"

#include <QGraphicsPathItem>

class WhiteCanvas;
class PositionBar;

class WhiteCanvasControl : public Control
{
    Q_OBJECT

public:
    WhiteCanvasControl(ResourceView * view, QGraphicsItem * canvas);

    virtual ~WhiteCanvasControl() override;

public slots:
    void setPosBarVisible(bool visible);

private:
    QGraphicsItem * create(ResourceView *res) override;

    virtual void resize(const QSizeF &size) override;

    virtual void sizeChanged() override;

private:
    void updatingTransform();

    void updateTransform();

private:
    PositionBar * posBar_;
};

class PositionBar : public QGraphicsPathItem
{
public:
    PositionBar(QGraphicsItem * parent = nullptr);

public:
    void update(QRectF const & viewRect, QRectF const & canvasRect, qreal scale, QPointF offset);
};

#endif // WHITECANVASCONTROL_H
