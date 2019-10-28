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

/*
 * WhiteCanvas is a container for all controls
 *  Controls are split into to groups, one with resource backend and another not
 *  Non-resource-backend controls are considered to be tools and are temporarily shown,
 *   SelectBox is one of them.
 */

class SHOWBOARD_EXPORT WhiteCanvas : public QObject, public QGraphicsRectItem
{
    Q_OBJECT
public:
    WhiteCanvas(QObject * parent = nullptr);

    virtual ~WhiteCanvas() override;

public slots:
    void switchPage(ResourcePage * page);

public:
    /*
     * add resource to attached resource page
     * use ResourcePage instead
     */
    void addResource(QUrl const & url);

    /*
     * add resource to attached resource page
     * use ResourcePage instead
     */
    void addResource(ResourceView * res);

    /*
     * show tool control of type @type
     *  these controls are not backed by resource and is single instance
     *  a new control of type @type is created if not exists in canvas
     */
    void showToolControl(QString const & type);

    /*
     * find control assosiate with resource @res
     */
    Control * findControl(ResourceView * res);

    /*
     * find control assosiate by @url
     *  @see ResourcePage::findResource
     */
    Control * findControl(QUrl const & url);

    /*
     * get top most control
     */
    Control * topControl();

public:
    void enableSelector(bool enable);

    /*
     * change canvas size
     */
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
