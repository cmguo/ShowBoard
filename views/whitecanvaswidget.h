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

    explicit WhiteCanvasWidget(WhiteCanvasWidget* mainView, QWidget *parent = nullptr);

    virtual ~WhiteCanvasWidget() override;

    static WhiteCanvasWidget * mainInstance();

public:
    WhiteCanvas * canvas()
    {
        return canvas_;
    }

    ResourcePackage * package();

    void setResourcePackage(ResourcePackage * pack);

private:
    virtual void resizeEvent(QResizeEvent *event) override;

    virtual void showEvent(QShowEvent *event) override;

    virtual bool eventFilter(QObject *watched, QEvent *event) override;

private:
    QGraphicsScene * scene_;
    WhiteCanvas * canvas_;
};

#endif // WHITECANVASWIDGET_H
