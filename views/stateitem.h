#ifndef STATEITEM_H
#define STATEITEM_H

#include <QGraphicsObject>

class SvgCache;
class QSvgRenderer;

class StateItem : public QGraphicsObject
{
    Q_OBJECT
public:
    enum State
    {
        Loading,
        Loaded,
        Failed,
    };

    Q_ENUM(State)

public:
    StateItem(QGraphicsItem * parent = nullptr);

public:
    void setLoading(QString const & title);

    void setLoaded(QString const & icon);

    void setFailed(QString const & msg);

    void updateTransform();

signals:
    void clicked();

private:
    void setSharedRenderer(QSvgRenderer * renderer);

    void setText(QString const & text);

private:
    virtual QRectF boundingRect() const override;

    virtual QPainterPath shape() const override;

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

    virtual void timerEvent(QTimerEvent * event) override;

    virtual void mousePressEvent(QGraphicsSceneMouseEvent * event) override;

    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent * event) override;

private:
    static SvgCache * cache_;
    static QSvgRenderer * loading_;
    static QSvgRenderer * failed_;

private:
    QGraphicsItem * iconItem_;
    QGraphicsItem * textItem_;
    QSvgRenderer * normal_;
    QSvgRenderer * hover_;
    QSvgRenderer * pressed_;

private:
    State state_;
    int timerId_;
    qreal rotate_;
};

#endif // STATEITEM_H
