#ifndef WHITECANVASCONTROL_H
#define WHITECANVASCONTROL_H

#include "core/control.h"

class WhiteCanvas;
class PositionBar;

class WhiteCanvasControl : public Control
{
    Q_OBJECT

    Q_PROPERTY(QString toolBarStyles WRITE setToolBarStyles)

public:
    WhiteCanvasControl(ResourceView * view, QGraphicsItem * canvas);

    virtual ~WhiteCanvasControl() override;

public:
    void setToolBarStyles(QString const & stylesheet);

public slots:
    void setPosBarVisible(bool visible);

public slots:
    void scaleUp();

    void scaleDown();

    void close();

private:
    QGraphicsItem * create(ResourceView *res) override;

    virtual void resize(const QSizeF &size) override;

    virtual void attached() override;

    virtual void sizeChanged() override;

    virtual void getToolButtons(QList<ToolButton *> &buttons, QList<ToolButton *> const & parents) override;

    virtual void getToolButtons(QList<ToolButton *> &buttons, ToolButton *parent) override;

    virtual void updateToolButton(ToolButton *button) override;

private:
    void updatingTransform();

    void updateTransform();

private:
    PositionBar * posBar_ = nullptr;
    QGraphicsItem* toolBar_ = nullptr;
};

#endif // WHITECANVASCONTROL_H
