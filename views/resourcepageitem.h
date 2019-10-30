#ifndef RESOURCEPAGEITEM_H
#define RESOURCEPAGEITEM_H

#include <QObject>
#include <QGraphicsRectItem>

class ResourceManager;
class ControlManager;
class ResourceView;
class ResourcePage;
class Control;

class ResourcePageItem : public QObject, public QGraphicsRectItem
{
    Q_OBJECT
public:
    ResourcePageItem(QGraphicsItem * parent = nullptr);

public:
    void switchPage(ResourcePage * page);

    Control * findControl(ResourceView * res);

    Control * findControl(QUrl const & url);

    void setGeometry(QRectF const & rect);

    ResourcePage * page()
    {
        return page_;
    }

private slots:
    void resourceInserted(QModelIndex const &parent, int first, int last);

    void resourceRemoved(QModelIndex const &parent, int first, int last);

    void resourceMoved(QModelIndex const &parent, int start, int end,
                       QModelIndex const &destination, int row);

private:
    void insertResource(int layer);

    void removeResource(int layer);

private:
    ResourceManager * resource_manager_;
    ControlManager * control_manager_;

protected:
    ResourcePage * page_;
};

#endif // RESOURCEPAGEITEM_H
