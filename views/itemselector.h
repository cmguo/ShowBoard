#ifndef ITEMSELECTOR_H
#define ITEMSELECTOR_H

#include "ShowBoard_global.h"

#include <QGraphicsRectItem>

class Control;
class SelectBox;
class WhiteCanvas;
class ToolbarWidget;

class SHOWBOARD_EXPORT ItemSelector : public QGraphicsRectItem
{
public:
    ItemSelector(QGraphicsItem * parent = nullptr);

public:
    void select(QGraphicsItem * item);

    void setForce(bool force);

    void autoTop(bool force);

    ToolbarWidget * toolBar();

    QGraphicsItem * selected()
    {
        return select_;
    }

private:
    friend class WhiteCanvas;

    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;

    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
    SelectBox * selBox_;

private:
    bool force_;
    bool autoTop_;

private:
    QGraphicsItem * select_;
    Control * selectControl_;

private:
    QPointF start_;
    QRectF direction_;

    enum SelectType
    {
        None = 0,
        Translate,
        Scale,
        Rotate,
        Canvas,
        TempNoMove,
        TempMoved
    };

    SelectType type_;
};

#endif // ITEMSELECTOR_H
