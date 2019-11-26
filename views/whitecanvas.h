#ifndef WHITECANVAS_H
#define WHITECANVAS_H

#include "ShowBoard_global.h"

#include <QObject>
#include <QGraphicsRectItem>

class ResourceView;
class ResourcePage;
class ResourcePackage;
class Control;
class ResourcePageItem;
class ToolBoxItem;
class ItemSelector;

class ToolButton;

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

public:
    ResourcePackage * package()
    {
        return package_;
    }

    void setResourcePackage(ResourcePackage * pack);

    void switchPage(ResourcePage * page);

public:
    /*
     * add resource to attached resource page
     * use ResourcePage instead
     */
    Control * addResource(QUrl const & url);

    /*
     * add resource to attached resource page
     * use ResourcePage instead
     */
    Control * addResource(ResourceView * res);

    /*
     * show tool control of type @type
     *  these controls are not backed by resource and is single instance
     *  a new control of type @type is created if not exists in canvas
     */
    void showToolControl(QString const & type);

    void hideToolControl(QString const & type);

    Control * getToolControl(QString const & type);

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

    ItemSelector * selector();

public:
    void enableSelector(bool enable);

    void moveSelectionTop(bool enable);

    /*
     * change canvas size
     */
    void setGeometry(QRectF const & rect);

private slots:
    void toolButtonClicked(QList<ToolButton *> const & buttons);

    void popupButtonsRequired(QList<ToolButton *> & buttons, QList<ToolButton *> const & parents);

private:
    virtual QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value) override;

    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;

    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
    ResourcePackage * package_;
    ResourcePageItem * canvas_;
    ToolBoxItem * tools_;
    ItemSelector * selector_;
};

#endif // WHITECANVAS_H
