#ifndef STATEITEM_H
#define STATEITEM_H

#include <QGraphicsSvgItem>

class SvgCache;
class QSvgRenderer;

class StateItem : public QGraphicsSvgItem
{
    Q_OBJECT
public:
    StateItem(QGraphicsItem * parent = nullptr);

    void setSvgFile(QString const & file, qreal rotate);

    void setSvgFiles(QString const & fileNormal, QString const & filePressed);

signals:
    void clicked();

private:
    void setSharedRenderer(QSvgRenderer * renderer);

    virtual void timerEvent(QTimerEvent * event) override;

    virtual void mousePressEvent(QGraphicsSceneMouseEvent * event) override;

    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent * event) override;

private:
    SvgCache * cache_;
    QSvgRenderer * normal_;
    QSvgRenderer * pressed_;
    int timerId_;
    qreal rotate_;
};

#endif // STATEITEM_H
