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
class WhiteCanvasControl;
class ToolButton;

/*
 * WhiteCanvas is a container for all controls
 *  Controls are split into to groups, one with resource backend and another not
 *  Non-resource-backend controls are considered to be tools and are temporarily shown,
 *   SelectBox is one of them.
 */

class SHOWBOARD_EXPORT WhiteCanvas : CANVAS_OBJECT public CanvasItem
{
    Q_OBJECT
public:
    static int THUMBNAIL_HEIGHT;

public:
    WhiteCanvas(QObject * parent = nullptr);

    virtual ~WhiteCanvas() override;

signals:
    void loadingChanged(bool loading);

    // include sub page
    void currentPageChanged(ResourcePage* page);

#ifdef PROD_TEST
public slots:
#else
public:
#endif
    ResourcePackage * package()
    {
        return package_;
    }

    ResourcePage * page();

    ResourcePage * subPage();

    void setResourcePackage(ResourcePackage * pack);

    void switchPage(ResourcePage * page);

    void updateThunmbnail();

    void showSubPages(bool show);

public:
    AnimCanvas * getDragAnimation(bool prev = false);

    void dragRelease();

#ifdef PROD_TEST
public slots:
#else
public:
#endif
    /*
     * add resource to attached resource page
     * use ResourcePage instead
     */
    Control * addResource(QUrl const & url, QVariantMap settings = {});

    Control * addResource(ResourceView * res);

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

#ifdef PROD_TEST
public slots:
#else
public:
#endif
    /*
     * show tool control of type @type
     *  these controls are not backed by resource and is single instance
     *  a new control of type @type is created if not exists in canvas
     */
    void showToolControl(QString const & typeOrUrl);

    void showToolControl(Control * control);

    void hideToolControl(QString const & typeOrUrl);

    void hideToolControl(Control * control);

    Control * getToolControl(QString const & typeOrUrl);

#ifdef PROD_TEST
public slots:
#else
public:
#endif
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

#ifdef PROD_TEST
public slots:
#else
public:
#endif
    void select(Control * control);

    Control * selected();

    Control * selectFirst();

    Control * selectNext();

    Control * selectPrev();

    Control * selectLast();

public:
    ItemSelector * selector();

    WhiteCanvasControl * canvasControl();

    void enableSelector(bool enable);

    bool loading();

    void onControlLoad(bool startOrFinished);

public:
    /*
     * change canvas size
     */
    void setGeometry(QRectF const & rect);

private:
#ifdef SHOWBOARD_QUICK
    virtual void itemChange(ItemChange change, const ItemChangeData &value) override;
#else
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

    virtual bool sceneEvent(QEvent *event) override;
#endif

    virtual bool event(QEvent *event) override;

private:
    Control * selectableNext(Control * control);

    Control * selectablePrev(Control * control);

private:
    ResourcePackage * package_;
    PageCanvas * canvas_;
    AnimCanvas * animCanvas_;
    PageCanvas * globalCanvas_;
    ToolCanvas * tools_;
    ItemSelector * selector_;
    WhiteCanvasControl * canvasControl_;
    int loadingCount_;
    int lastPage_;
};

#endif // WHITECANVAS_H
