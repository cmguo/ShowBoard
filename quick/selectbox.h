#ifndef SELECTBOXQUICKITEM_H
#define SELECTBOXQUICKITEM_H

#include <QQuickItem>

class SelectBox : public QQuickItem
{
    Q_OBJECT
public:
    SelectBox(QQuickItem * parent = nullptr);

public:
    void setRect(QRectF const & rect);

    void setVisible(bool select, bool scale = false, bool scale2 = false, bool rotate = false, bool mask = false);

    int hitTest(QPointF const & pos, QRectF & direction);

    QRectF boundRect() const;

private:
    QQuickItem * rotate_;
    QQuickItem * leftTop_;
    QQuickItem * rightTop_;
    QQuickItem * rightBottom_;
    QQuickItem * leftBottom_;
    QQuickItem * left_;
    QQuickItem * top_;
    QQuickItem * right_;
    QQuickItem * bottom_;
};

#endif // SELECTBOXQUICKITEM_H
