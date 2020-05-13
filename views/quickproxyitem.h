#ifndef QUICKPROXYITEM_H
#define QUICKPROXYITEM_H

#include "ShowBoard_global.h"

#include <QQuickItem>

class QQuickWidget;
class QWidget;

class SHOWBOARD_EXPORT QuickProxyItem : public QQuickItem
{
    Q_OBJECT
public:
    QuickProxyItem(QQuickWidget * quickWidget);

    virtual ~QuickProxyItem() override;

public slots:
    void updateState();

protected:
    virtual void onGeometryChanged(QRect const & newGeometry);

    virtual void onActiveChanged(bool active);

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

private:
    QQuickWidget* quickwidget_;
    QWidget* commonParent_;
    bool active_ = false;
};

#endif // QUICKPROXYITEM_H
