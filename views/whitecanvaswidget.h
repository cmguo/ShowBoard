#ifndef WHITECANVASWIDGET_H
#define WHITECANVASWIDGET_H

#include "ShowBoard_global.h"

#include <QGraphicsView>

class QGraphicsScene;
class WhiteCanvas;
class ResourcePackage;

class SHOWBOARD_EXPORT WhiteCanvasWidget : public QGraphicsView
{
    Q_OBJECT
public:
    explicit WhiteCanvasWidget(QWidget *parent = nullptr);

    virtual ~WhiteCanvasWidget() override;

    static WhiteCanvasWidget * mainInstance();

public:
    WhiteCanvas * canvas()
    {
        return canvas_;
    }

    ResourcePackage * package()
    {
        return package_;
    }

    void setResourcePackage(ResourcePackage * pack);

signals:

public slots:

private:
    virtual void resizeEvent(QResizeEvent *event) override;

private:
    QGraphicsScene * scene_;
    WhiteCanvas * canvas_;
    ResourcePackage * package_;
};

#endif // WHITECANVASWIDGET_H
