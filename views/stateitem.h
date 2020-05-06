#ifndef STATEITEM_H
#define STATEITEM_H

#include <QGraphicsObject>
#include <QPen>

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
    StateItem(QGraphicsItem * parent = nullptr);

public:
    void setLoading();

    void setLoading(QString const & msg);

    void setLoaded(QString const & icon);

    void setFailed(QString const & error);

    void updateTransform();

    bool hitTest(QGraphicsItem * child, const QPointF &pt);

signals:
    void clicked();

private:
    void decideStyles(bool independent, bool istool);

    void setSvg(QSvgRenderer * renderer);

    void setMovie(QMovie * movie);

    void setText(QString const & text);

    void updateLayout();

public:
    virtual QRectF boundingRect() const override;

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

    virtual void timerEvent(QTimerEvent * event) override;

    virtual void mousePressEvent(QGraphicsSceneMouseEvent * event) override;

    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent * event) override;

    virtual bool sceneEvent(QEvent *event) override;

private:
    QGraphicsItem* createIconItem(bool independent);

    QGraphicsItem* createTextItem(bool independent);

    QGraphicsItem* createButtonItem(bool independent);

private:
    static SvgCache * cache_;
    static QMovie * loading2_;
    static QSvgRenderer * loading_;
    static QSvgRenderer * failed_;

private:
    QGraphicsItem * iconItem_;
    QGraphicsItem * textItem_;
    QGraphicsItem * btnItem_;
    QSvgRenderer * normal_;
    QSvgRenderer * hover_;
    QSvgRenderer * pressed_;

private:
    QString title_;
    State state_;
    QRectF rect_;
    bool fixedSize_ = false;

private:
    // styles
    QPen pen_;
    QBrush brush_;
    qreal borderRadius_;
    QColor textColor1_;
    QColor textColor2_;
    QString textSize1_;
    QString textSize2_;

    bool showBackground_;
    int timerId_;
    int animate_;
    int touchId_;
};

#endif // STATEITEM_H
