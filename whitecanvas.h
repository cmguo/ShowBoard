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
class Control;

struct ToolButton;

class SHOWBOARD_EXPORT WhiteCanvas : public QObject, public QGraphicsRectItem
{
    Q_OBJECT
public:
    WhiteCanvas(QObject * parent = nullptr);

    virtual ~WhiteCanvas() override;

public slots:
    void switchPage(ResourcePage * page);

public:
    void addResource(QUrl const & url);

    void addResource(ResourceView * res);

    Control * findControl(ResourceView * res);

    Control * findControl(QUrl const & url);

    Control * topControl();

public:
    void enableSelector(bool enable);

    void setGeometry(QRectF const & rect);

private slots:
    void resourceInserted(QModelIndex const &parent, int first, int last);

    void resourceRemoved(QModelIndex const &parent, int first, int last);

    void resourceMoved(QModelIndex const &parent, int start, int end,
                       QModelIndex const &destination, int row);

    void toolButtonClicked(ToolButton * button);

private:
    void insertResource(int layer);

    void removeResource(int layer);

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
