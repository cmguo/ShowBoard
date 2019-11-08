#ifndef SELECTBAR_H
#define SELECTBAR_H

#include <QGraphicsRectItem>

class SelectBar : public QGraphicsRectItem
{
public:
    SelectBar(QGraphicsItem * item, QGraphicsItem * parent = nullptr);

public:
    void setSelected(bool selected);

    void updateRect();

private:
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

private:
    QGraphicsItem * item_;
    bool selected_;
};

#endif // SELECTBAR_H
