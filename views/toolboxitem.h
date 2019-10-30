#ifndef TOOLBOXITEM_H
#define TOOLBOXITEM_H

#include "resourcepageitem.h"

class ToolBoxItem : public ResourcePageItem
{
public:
    ToolBoxItem(QGraphicsItem * parent = nullptr);

public:
    void showToolControl(QString const & type);

    void hideToolControl(QString const & type);

    Control * getToolControl(QString const & type);

private:
    void showItem(QGraphicsItem * item);

    void hideItem(QGraphicsItem * item);

    virtual QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value) override;

private:
    QGraphicsItem * shown_;
};

#endif // TOOLBOXITEM_H
