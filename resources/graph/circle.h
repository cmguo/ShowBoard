#ifndef CIRCLE_H
#define CIRCLE_H

#include "ellipse.h"

class Circle : public Ellipse
{
    Q_OBJECT
public:
    Circle();

    Q_INVOKABLE Circle(Resource * res);

    Circle(QPointF const & pt);

    Circle(Circle const & o);

public:
    virtual QPainterPath path() override;
};

#endif // CIRCLE_H
