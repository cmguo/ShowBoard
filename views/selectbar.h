#ifndef SELECTBAR_H
#define SELECTBAR_H

#include <QGraphicsRectItem>

class SelectBar : public QGraphicsRectItem
{
public:
    static constexpr qreal HEIGHT = 48.0;

    SelectBar(QGraphicsItem * item, QGraphicsItem * parent = nullptr);

public:
    void setSelected(bool selected);

    void setRect(QRectF const & rect);

    void updateRect();

    void updateRectFromChild(QRectF & rect);

    void updateRectToChild(QRectF & rect);

    void updateTransform();

    void update();

private:
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

private:
    QGraphicsItem * item_;
    bool selected_;
};

#endif // SELECTBAR_H
