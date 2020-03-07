#include "whitecanvas.h"
#include "whitecanvastools.h"

#include "core/resourcepage.h"
#include "core/resourcepackage.h"
#include "core/toolbutton.h"

#include <QApplication>
#include <QListView>
#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsView>

static constexpr char const * toolstr =
        "new|新建|:/showboard/icons/page.new.png,normal=,disabled=.disabled;"
        "next||:/showboard/icons/page.next.png,normal=,disabled=.disabled;"
        "page|0/0|;"
        "prev||:/showboard/icons/page.prev.png,normal=,disabled=.disabled;"
        ;

WhiteCanvasTools::WhiteCanvasTools()
    : canvas_(nullptr)
    , pageList_(nullptr)
{
    setToolsString(toolstr);
    followTrigger();
}

void WhiteCanvasTools::attachToWhiteCanvas(WhiteCanvas *whiteCanvas)
{
    canvas_ = whiteCanvas;
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
        QPoint pos(200, 100);
        ToolButton* button = getStringButton(1);
        if (button->associatedWidgets().isEmpty()) {
            pageList_->setParent(canvas_->scene()->views().first()->parentWidget());
        } else {
            QWidget* btn = button->associatedWidgets().first();
            pageList_->setParent(btn->window());
            pos = btn->mapTo(pageList_->parentWidget(), QPoint());
            pos += QPoint(pageList_->width() / 2 - btn->width() / 2,
                          -pageList_->sizeHint().height());
        }
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

void WhiteCanvasTools::gotoPage(int n)
{
    canvas_->package()->switchPage(n);
}

void WhiteCanvasTools::setOption(const QByteArray &key, QVariant value)
{
    canvas_->setProperty("FromUser", true);
    if (key == "new")
        newPage();
    else if (key == "prev")
        prevPage();
    else if (key == "list")
        pageList();
    else if (key == "next")
        nextPage();
    else if (key == "goto")
        gotoPage(value.toInt());
    else
        ToolButtonProvider::setOption(key, value);
    canvas_->setProperty("FromUser", QVariant());
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
        QRect rect = option.rect;
        painter->drawText(rect.topLeft() + QPoint(5, 20), QString("%1").arg(index.row() + 1));
        rect.adjust(24, 4, -4, -4);
        Qt::CheckState checked = index.data(Qt::CheckStateRole).value<Qt::CheckState>();
        if (checked != Qt::Unchecked) {
            painter->setPen(QPen(Qt::blue, 2));
            painter->drawRoundedRect(rect.adjusted(-3, -3, 3, 3), 3, 3);
        }
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
    list->setFixedHeight(500);
    QObject::connect(list, &QListView::activated, this, [this](QModelIndex i) {
       setOption("goto", i.row());
    });
    list->installEventFilter(this);
    return list;
}

bool WhiteCanvasTools::eventFilter(QObject * watched, QEvent *event)
{
    QWidget* widget = qobject_cast<QWidget*>(watched);
    if (event->type() == QEvent::Show) {
        widget->setFocus();
    }
    if (event->type() == QEvent::FocusOut) {
        if (QApplication::focusWidget()
                && QApplication::focusWidget()->parent() == watched) {
            widget->setFocus();
        } else {
            widget->hide();
        }
    }
    return false;
}
