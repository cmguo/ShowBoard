#ifndef DRAWINGTOOL_H
#define DRAWINGTOOL_H

#include "core/control.h"

#include <QUrl>

class DrawingTool : public Control
{
    Q_OBJECT

    Q_PROPERTY(QUrl newUrl MEMBER newUrl_)
public:
    Q_INVOKABLE DrawingTool(ResourceView *res);

public:
    Control * newControl();

    void finishControl(Control * control);

private:
    virtual QGraphicsItem * create(ResourceView *res) override;

private:
    QUrl newUrl_;
};

#endif // DRAWINGTOOL_H
