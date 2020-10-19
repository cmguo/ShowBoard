#ifndef WHITECANVASCONTROL_H
#define WHITECANVASCONTROL_H

#include "core/control.h"

class WhiteCanvas;
class PositionBar;
class WhiteCanvas;

class WhiteCanvasControl : public Control
{
    Q_OBJECT

    Q_PROPERTY(QString toolBarStyles WRITE setToolBarStyles)
    Q_PROPERTY(bool noScaleButton WRITE setNoScaleButton)

public:
    static constexpr Flag NoScaleButton = CustomFlag;

    WhiteCanvasControl(ResourceView * view, QGraphicsItem * canvas);

    // fake canvas control
    WhiteCanvasControl(WhiteCanvas * canvas);

    virtual ~WhiteCanvasControl() override;

public:
    void setToolBarStyles(QString const & stylesheet);

    void setNoScaleButton(bool b);

public slots:
    void setPosBarVisible(bool visible);

public slots:
    void scaleUp();

    void scaleDown();

    void close();

public:
    PositionBar * posBar() const { return posBar_; }

    QGraphicsItem* toolBar() const { return toolBar_; }

private:
    QGraphicsItem * create(ResourceView *res) override;

    virtual void resize(const QSizeF &size) override;

    virtual void attached() override;

    virtual void sizeChanged() override;

    virtual void adjusting(bool be) override;

    virtual void move(QPointF &delta) override;

    virtual void gesture(GestureContext *ctx, QPointF const &to1, QPointF const &to2) override;

public:
    virtual void getToolButtons(QList<ToolButton *> &buttons, QList<ToolButton *> const & parents) override;

    virtual void getToolButtons(QList<ToolButton *> &buttons, ToolButton *parent) override;

    virtual void updateToolButton(ToolButton *button) override;

private:
    void updatingTransform();

    void updateTransform();

    WhiteCanvas * whiteCanvas();

    void pageSwitchStart(QPointF const & delta);

    bool pageSwitchMove(QPointF const & delta);

    bool pageSwitchEnd(bool cancel);

private:
    PositionBar * posBar_ = nullptr;
    QGraphicsItem* toolBar_ = nullptr;
    QObject * pageSwitch_ = nullptr;
};

#endif // WHITECANVASCONTROL_H
