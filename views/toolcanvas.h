#ifndef TOOLCANVAS_H
#define TOOLCANVAS_H

#include "pagecanvas.h"

class ToolCanvas : public PageCanvas
{
public:
    ToolCanvas(QGraphicsItem * parent = nullptr);

public:
    void showToolControl(Control * control);

    void hideToolControl(Control * control);

    Control * getToolControl(QString const & typeOrUrl);

private:
    void showItem(QGraphicsItem * item);

    void hideItem(QGraphicsItem * item);

    virtual QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value) override;

private:
    QGraphicsItem * shown_;
};

#endif // TOOLCANVAS_H
