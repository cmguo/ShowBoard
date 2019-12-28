#ifndef IMAGECONTROL_H
#define IMAGECONTROL_H

#include "core/control.h"

class QPixmap;

class ImageControl : public Control
{
    Q_OBJECT
public:
    Q_INVOKABLE ImageControl(ResourceView *res);

protected:
    virtual QGraphicsItem * create(ResourceView * res) override;

    virtual void attached() override;

    virtual void onData(QByteArray data) override;

    virtual void detached() override;
};

#endif // IMAGECONTROL_H
