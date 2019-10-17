#ifndef WHITECANVAS_H
#define WHITECANVAS_H

#include "ShowBoard_global.h"

#include <QGraphicsRectItem>

class ItemSelector;
class QComponentContainer;
class ResourceManager;
class ControlManager;
class ResourceView;
class WhitePage;

class SHOWBOARD_EXPORT WhiteCanvas : public QGraphicsRectItem
{
public:
    WhiteCanvas();

    virtual ~WhiteCanvas() override;

public:
    void switchPage(WhitePage * page);

    void addResource(QUrl const url);

    void addResource(ResourceView * res, bool fromSwitch = false);

    void copyResource(QGraphicsItem * item);

    void removeResource(QGraphicsItem * item);

private:
    static QComponentContainer & containter();

private:
    ResourceManager * resource_manager_;
    ControlManager * control_manager_;

private:
    QGraphicsRectItem * canvas_;
    ItemSelector * selector_;
    WhitePage * page_;
};

#endif // WHITECANVAS_H
