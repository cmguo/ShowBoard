#ifndef TOOLCANVAS_H
#define TOOLCANVAS_H

#include "pagecanvas.h"
#include "core/controlview.h"

class ToolCanvas : public PageCanvas
{
public:
    ToolCanvas(CanvasItem * parent = nullptr);

public:
    void showToolControl(Control * control);

    void hideToolControl(Control * control);

    Control * getToolControl(QString const & typeOrUrl);

private:
    void showItem(ControlView * item);

    void hideItem(ControlView * item);

#ifdef SHOWBOARD_QUICK
    virtual void itemChange(ItemChange change, const ItemChangeData &value) override;
#else
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
#endif

private:
    ControlView * shown_;
};

#endif // TOOLCANVAS_H
