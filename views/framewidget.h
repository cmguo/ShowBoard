#ifndef FRAMEWIDGET_H
#define FRAMEWIDGET_H

#include "ShowBoard_global.h"

#include <QWidget>

class ResourceView;

class SHOWBOARD_EXPORT FrameWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FrameWidget(QWidget * content, QWidget *parent = nullptr);

public:
    QWidget * content();

    void setBorderRadius(int size, int radius, int padding = 0);

    void setArrowSize(QSize const & size);

    void setArrowPosition(QPoint pos, int dir, int off);

protected:
    virtual void paintEvent(QPaintEvent *event) override;

    virtual bool eventFilter(QObject* watched, QEvent *event) override;

private:
    void updateShape();

    static QPainterPath toRoundPolygon(QPolygonF const & polygon, QVector<qreal> const & radiuses);

private:
    QWidget * content_;
    QPainterPath path_;
    int borderSize_ = 0;
    int borderRadius_ = 0;
    int paddingSize_ = 0;
    QSize arrowSize_;
    QPoint arrowPos_;
    int arrowDir_;
    int arrowOff_;
};

#endif // FRAMEWIDGET_H
