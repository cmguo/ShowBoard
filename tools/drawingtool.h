#ifndef DRAWINGTOOL_H
#define DRAWINGTOOL_H

#include "core/control.h"

#include <QUrl>
#include <QVariantMap>
#include <QColor>

class DrawingTool : public Control
{
    Q_OBJECT

    Q_PROPERTY(QUrl newUrl MEMBER newUrl_)
    Q_PROPERTY(QVariantMap newSettings MEMBER newSettings_)
    Q_PROPERTY(bool translucent READ translucent WRITE setTranslucent)
    Q_PROPERTY(QColor color READ color WRITE setColor)
    Q_PROPERTY(qreal width READ width WRITE setWidth)

public:
    Q_INVOKABLE DrawingTool(ResourceView *res);

public:
    QColor color() const;

    void setColor(QColor color);

    qreal width() const;

    void setWidth(qreal width);

public:
    Control * newControl();

    void removeControl(Control * control);

    void finishControl(Control * control);

    bool translucent() const;

    void setTranslucent(bool on);

signals:
    void controlCreated(Control * control);

    void drawFinished(bool done);

private:
    virtual QGraphicsItem * create(ResourceView *res) override;

    virtual void resize(const QSizeF &size) override;

private:
    QUrl newUrl_;
    QVariantMap newSettings_;
};

#endif // DRAWINGTOOL_H
