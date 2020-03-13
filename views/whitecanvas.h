#ifndef WHITECANVAS_H
#define WHITECANVAS_H

#include "ShowBoard_global.h"
#include "canvasitem.h"

#include <QObject>

class ResourceView;
class ResourcePage;
class ResourcePackage;
class Control;
class PageCanvas;
class AnimCanvas;
class ToolCanvas;
class ItemSelector;

class ToolButton;

/*
 * WhiteCanvas is a container for all controls
 *  Controls are split into to groups, one with resource backend and another not
 *  Non-resource-backend controls are considered to be tools and are temporarily shown,
 *   SelectBox is one of them.
 */

class SHOWBOARD_EXPORT WhiteCanvas : public QObject, public CanvasItem
{
    Q_OBJECT
public:
    static constexpr int THUMBNAIL_HEIGHT = 108;

public:
    WhiteCanvas(QObject * parent = nullptr);

    virtual ~WhiteCanvas() override;

signals:
    void loadingChanged(bool loading);

public:
    ResourcePackage * package()
    {
        return package_;
    }

    ResourcePage* page();

    void setResourcePackage(ResourcePackage * pack);

    void switchPage(ResourcePage * page);

    void updateThunmbnail();

public:
    /*
     * add resource to attached resource page
     * use ResourcePage instead
     */
    Control * addResource(QUrl const & url, QVariantMap settings = {});

    /*
     * copy resource to attached resource page
     * use ResourcePage instead
     */
    Control * copyResource(Control * control);

    /*
     * add resource to attached resource page
     * use ResourcePage instead
     */
    void removeResource(Control * control);

public:
    /*
     * show tool control of type @type
     *  these controls are not backed by resource and is single instance
     *  a new control of type @type is created if not exists in canvas
     */
    void showToolControl(QString const & type);

    void showToolControl(Control * control);

    void hideToolControl(QString const & type);

    void hideToolControl(Control * control);

    Control * getToolControl(QString const & type);

public:
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

    void select(Control * control);

    Control * selected();

    Control * selectFirst();

    Control * selectNext();

    Control * selectPrev();

    Control * selectLast();

public:
    ItemSelector * selector();

    void enableSelector(bool enable);

    bool loading();

    void onControlLoad(bool startOrFinished);

public:
    /*
     * change canvas size
     */
    void setGeometry(QRectF const & rect);

private slots:
    void toolButtonClicked(QList<ToolButton *> const & buttons);

    void popupButtonsRequired(QList<ToolButton *> & buttons, QList<ToolButton *> const & parents);

private:
    virtual QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value) override;

    virtual bool sceneEvent(QEvent *event) override;

/*
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;

    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
*/
private:
    QGraphicsItem * selectableNext(QGraphicsItem * item);

    QGraphicsItem * selectablePrev(QGraphicsItem * item);

private:
    ResourcePackage * package_;
    PageCanvas * canvas_;
    AnimCanvas * animCanvas_;
    PageCanvas * globalCanvas_;
    ToolCanvas * tools_;
    ItemSelector * selector_;
    int loadingCount_;
    int lastPage_;
};

#endif // WHITECANVAS_H
