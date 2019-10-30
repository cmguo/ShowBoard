#ifndef SQUARE_H
#define SQUARE_H

#include "rectangle.h"

class Square : public Rectangle
{
    Q_OBJECT
public:
    Q_INVOKABLE Square(Resource * res);

    Square(QPointF const & pt);

    Square(Square const & o);

public:
    virtual void movePoint(QPointF const & pt) override;
};

#endif // SQUARE_H
