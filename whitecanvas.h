#ifndef WHITECANVAS_H
#define WHITECANVAS_H

#include "ShowBoard_global.h"

#include <QObject>
#include <QGraphicsRectItem>

class ItemSelector;
class QComponentContainer;
class ResourceManager;
class ControlManager;
class ResourceView;
class ResourcePage;

class SHOWBOARD_EXPORT WhiteCanvas : public QObject, public QGraphicsRectItem
{
    Q_OBJECT
public:
    WhiteCanvas(QObject * parent = nullptr);

    virtual ~WhiteCanvas() override;

public slots:
    void switchPage(ResourcePage * page);

    void addResource(QUrl const & url);

    void addResource(ResourceView * res, bool fromSwitch = false);

    void copyResource(QGraphicsItem * item);

    void removeResource(QGraphicsItem * item, bool fromSwitch = false);

    void enableSelector(bool enable);

private:
    virtual QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value) override;

private:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;

    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
    ResourceManager * resource_manager_;
    ControlManager * control_manager_;

private:
    QGraphicsRectItem * canvas_;
    ItemSelector * selector_;
    ResourcePage * page_;
};

#endif // WHITECANVAS_H
