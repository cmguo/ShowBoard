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
        None,
        Loading,
        Loaded,
        Failed,
    };

    Q_ENUM(State)

public:
    StateItem(QString const & title, QGraphicsItem * parent = nullptr);

public:
    void showBackground(bool show);

    void setLoading();

    void setLoaded(QString const & icon);

    void setFailed(QString const & error);

    void updateTransform();

    bool hitTest(QGraphicsItem * child, const QPointF &pt);

signals:
    void clicked();

private:
    void setSharedRenderer(QSvgRenderer * renderer);

    void setText(QString const & text);

public:
    virtual QRectF boundingRect() const override;

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

    virtual void timerEvent(QTimerEvent * event) override;

    virtual void mousePressEvent(QGraphicsSceneMouseEvent * event) override;

    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent * event) override;

    virtual bool sceneEvent(QEvent *event) override;

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
    QString title_;
    bool showBackground_;
    int timerId_;
    qreal rotate_;
    int touchId_;
};

#endif // STATEITEM_H
