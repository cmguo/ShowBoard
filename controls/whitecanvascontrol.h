#ifndef WHITECANVASCONTROL_H
#define WHITECANVASCONTROL_H

#include "core/control.h"

class WhiteCanvas;

class WhiteCanvasControl : public Control
{
public:
    WhiteCanvasControl(ResourceView * view, QGraphicsItem * canvas);

    virtual ~WhiteCanvasControl() override;

private:
    QGraphicsItem * create(ResourceView *res) override;

    virtual void resize(const QSizeF &size) override;

    virtual void updateTransform() override;
};

#endif // WHITECANVASCONTROL_H
