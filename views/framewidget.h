#ifndef FRAMEWIDGET_H
#define FRAMEWIDGET_H

#include "ShowBoard_global.h"

#include <QBitmap>
#include <QPainterPath>
#include <QWidget>

class ResourceView;

class SHOWBOARD_EXPORT FrameWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FrameWidget(QWidget * content, QWidget *parent = nullptr);

public:
    QWidget * content();

    void setBackground(QColor const & color);

    void setBorder(QColor const & color, int size = 1, int radius = 8, int padding = 0);

    void setArrowSize(QSize const & size);

    void setArrowPosition(QPoint pos, int dir, int off);

protected:
    virtual void paintEvent(QPaintEvent *event) override;

    virtual bool eventFilter(QObject* watched, QEvent *event) override;

private:
    void updateShape();

    static QBitmap roundMask(QRect const & rect, int radius);

    static QPainterPath toRoundPolygon(QPolygonF const & polygon, QVector<qreal> const & radiuses);

private:
    enum Flags {
        BackgroundSet = 1
    };

private:
    QWidget * content_;
    QPainterPath path_;
    QBitmap mask_;
    int flags_ = 0;
    QColor backgroundColor_;
    QColor borderColor_;
    int borderSize_ = 0;
    int borderRadius_ = 0;
    int paddingSize_ = 0;
    QSize arrowSize_;
    QPoint arrowPos_;
    int arrowDir_;
    int arrowOff_;
};

#endif // FRAMEWIDGET_H
