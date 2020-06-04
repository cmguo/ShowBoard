#ifndef QUICKPROXYITEM_H
#define QUICKPROXYITEM_H

#include "ShowBoard_global.h"

#include <QQuickItem>

class QQuickWidget;
class QWidget;

class SHOWBOARD_EXPORT QuickProxyItem : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(QQuickWidget* quickWidget WRITE setQuickWidget)
public:
    QuickProxyItem(QQuickWidget * quickWidget, QQuickItem * parent = nullptr);

    virtual ~QuickProxyItem() override;

public:
    void setQuickWidget(QQuickWidget * quickWidget);

public slots:
    void updateState();

protected:
    virtual void onGeometryChanged(QRect const & newGeometry);

    virtual void onActiveChanged(bool active);

    virtual void quickWidgetChanged(QQuickWidget* quickwidget) { (void) quickwidget; }

    bool isActive() const { return active_; }

    void setCommonParent(QWidget * widget);

private:
    virtual void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry) override;

    virtual void itemChange(ItemChange change, const ItemChangeData & value) override;

private:
    void setGeometry(const QRect &newGeometry);

    void setActive(bool active);

    void addOverlayItemRegion(QRegion & region);

    static void addOverlayItemRegion(QRegion & region, QQuickItem* item);

protected:
    QQuickWidget* quickWidget_;

private:
    QWidget* commonParent_;
    bool active_ = false;
};

#endif // QUICKPROXYITEM_H
