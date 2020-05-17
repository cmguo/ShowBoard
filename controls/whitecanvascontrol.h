#ifndef WHITECANVASCONTROL_H
#define WHITECANVASCONTROL_H

#include "core/control.h"

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

public slots:
    void scaleUp();

    void scaleDown();

private:
    QGraphicsItem * create(ResourceView *res) override;

    virtual void resize(const QSizeF &size) override;

    virtual void attached() override;

    virtual void sizeChanged() override;

    virtual void getToolButtons(QList<ToolButton *> &buttons, ToolButton *parent) override;

    virtual void updateToolButton(ToolButton *button) override;

private:
    void updatingTransform();

    void updateTransform();

private:
    PositionBar * posBar_;
    QGraphicsItem* toolBar_;
};

#endif // WHITECANVASCONTROL_H
