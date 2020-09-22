#ifndef FRAMEWIDGET_H
#define FRAMEWIDGET_H

#include "ShowBoard_global.h"

#include <QRegion>
#include <QPainterPath>
#include <QWidget>

class ResourceView;
class FrameWidget;

class FrameWidgetShadow : public QWidget {
    Q_OBJECT

public:
    FrameWidgetShadow(FrameWidget* parent);

protected:
    virtual void paintEvent(QPaintEvent *event) override;

private:
    FrameWidget* frameWidget_;
};


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
    virtual bool eventFilter(QObject* watched, QEvent *event) override;

private:
    void updateShape();

    static QRegion roundMask(QRect const & rect, int radius);

    static QPainterPath toRoundPolygon(QPolygonF const & polygon, QVector<qreal> const & radiuses);

private:
    enum Flags {
        BackgroundSet = 1
    };

private:
    QWidget * content_;
    QPainterPath path_;
    QRegion mask_;
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

    int shadowRadius_ = 24;

    friend class FrameWidgetShadow;

    FrameWidgetShadow* shadow_;
};

#endif // FRAMEWIDGET_H
