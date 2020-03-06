#include "whitecanvas.h"
#include "whitecanvastools.h"

#include "core/resourcepage.h"
#include "core/resourcepackage.h"
#include "core/toolbutton.h"

#include <QApplication>
#include <QListView>
#include <QPainter>

static constexpr char const * toolstr =
        "newPage()|新建|:/showboard/icons/page.new.png,normal=,disabled=.disabled;"
        "nextPage()||:/showboard/icons/page.next.png,normal=,disabled=.disabled;"
        "pageList()|0/0|;"
        "prevPage()||:/showboard/icons/page.prev.png,normal=,disabled=.disabled;"
        ;

WhiteCanvasTools::WhiteCanvasTools()
{
    setToolsString(toolstr);
    followTrigger();
}

void WhiteCanvasTools::attachToWhiteCanvas(WhiteCanvas *whiteCanvas)
{
    ResourcePackage * package = whiteCanvas->package();
    QObject::connect(package, &ResourcePackage::pageCountChanged,
                     this, &WhiteCanvasTools::update);
    QObject::connect(package, &ResourcePackage::currentPageChanged,
                     this, &WhiteCanvasTools::update);
    update();
}

void WhiteCanvasTools::newPage()
{
    canvas_->package()->newPage();
}

void WhiteCanvasTools::prevPage()
{
    canvas_->package()->gotoPrevious();
}

void WhiteCanvasTools::pageList()
{
    if (pageList_ == nullptr) {
        pageList_ = createPageList(canvas_->package());
        QWidget * window = nullptr;
        pageList_->setParent(window);
        QPoint pos;// = btn->mapTo(window, QPoint());
        pos -= QPoint(0, pageList_->sizeHint().height());
        pageList_->move(pos);
    }
    if (!pageList_->isVisible()) {
        canvas_->updateThunmbnail();
        QModelIndex index(canvas_->package()->currentModelIndex());
        qobject_cast<QListView*>(pageList_)->scrollTo(index, QListView::PositionAtCenter);
    }
    pageList_->setVisible(!pageList_->isVisible());
}

void WhiteCanvasTools::nextPage()
{
    canvas_->package()->gotoNext();
}

void WhiteCanvasTools::update()
{
    QList<ToolButton*> buttons;
    getToolButtons(buttons);
    int total = canvas_->package()->pageCount();
    int index = canvas_->package()->currentIndex();
    buttons[1]->setEnabled(index > 0);
    buttons[2]->setText(QString("%1/%2").arg(index + 1).arg(total));
    buttons[3]->setEnabled(index + 1 < total);
}

class PageDelegate : public QAbstractItemDelegate
{
    // QAbstractItemDelegate interface
public:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        painter->save();
        painter->setBrush(index.data(Qt::BackgroundRole).value<QBrush>());
        QRect rect = option.rect;
        painter->drawRoundedRect(rect.adjusted(1, 1, -1, -1), 3, 3);
        painter->drawText(rect.topLeft() + QPoint(5, 20), QString("%1").arg(index.row() + 1));
        rect.adjust(24, 4, -4, -4);
        ResourcePage * page = index.data().value<ResourcePage*>();
        painter->drawPixmap(rect, page->thumbnail());
        painter->restore();
    }

    QSize sizeHint(const QStyleOptionViewItem &, const QModelIndex &index) const
    {
        ResourcePage * page = index.data().value<ResourcePage*>();
        return page->thumbnail().size() + QSize(28, 8);
    }
};

QWidget *WhiteCanvasTools::createPageList(ResourcePackage * package)
{
    QListView * list = new QListView;
    list->setModel(package);
    list->setStyleSheet("border:1px blue");
    list->setItemDelegate(new PageDelegate);
    list->setVerticalScrollMode(QListView::ScrollPerPixel);
    list->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    list->setMinimumHeight(50);
    list->setMaximumHeight(500);
    QObject::connect(list, &QListView::activated, [package](QModelIndex i) {
       package->switchPage(i.row());
    });
    return list;
}
