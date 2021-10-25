#ifndef STATEITEM_H
#define STATEITEM_H

#include "ShowBoard_global.h"

#include <QQuickItem>

class SvgCache;
class QSvgRenderer;

class SHOWBOARD_EXPORT StateItem : public QQuickItem
{
    Q_OBJECT
public:
    enum State
    {
        None,
        Loading,
        Loaded,
        Failed,
    };

    Q_ENUM(State)

public:
    StateItem(QQuickItem * parent = nullptr);

public:
    void setLoading();

    void setLoading(QString const & msg);

    void setLoaded(QString const & icon);

    void setFailed(QString const & error);

    void updateTransform();

    bool hitTest(QQuickItem * child, const QPointF &pt);

signals:
    void clicked();

public:
    virtual QRectF boundingRect() const override;

private:
    QString title_;
    State state_;
    QRectF rect_;
    bool fixedSize_ = false;

private:
    // styles
    qreal borderRadius_;
    QColor textColor1_;
    QColor textColor2_;
    QString textSize1_;
    QString textSize2_;

    bool showBackground_;
};

#endif // STATEITEM_H
