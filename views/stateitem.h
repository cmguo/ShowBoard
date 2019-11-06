#ifndef STATEITEM_H
#define STATEITEM_H

#include <QGraphicsSvgItem>

class SvgCache;
class QSvgRenderer;

class StateItem : public QGraphicsSvgItem
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
    void setLoading();

    void setLoaded(QString const & icon);

    void setFailed(QString const & msg);

    void setScale(qreal sx, qreal sy);

signals:
    void clicked();

private:
    void setSharedRenderer(QSvgRenderer * renderer);

    virtual void timerEvent(QTimerEvent * event) override;

    virtual void mousePressEvent(QGraphicsSceneMouseEvent * event) override;

    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent * event) override;

private:
    static SvgCache * cache_;
    static QSvgRenderer * loading_;
    static QSvgRenderer * failed_;

private:
    QSvgRenderer * normal_;
    QSvgRenderer * hover_;
    QSvgRenderer * pressed_;

private:
    State state_;
    int timerId_;
    qreal rotate_;
};

#endif // STATEITEM_H
