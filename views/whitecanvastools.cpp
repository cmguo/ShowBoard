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
#include <QQuickWidget>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickImageProvider>

static constexpr char const * toolstr =
        "new|新建|:/showboard/icons/page.new.svg;"
        "prev|上一页|:/showboard/icons/page.prev.svg;"
        "page|当前页|;"
        "next|下一页|:/showboard/icons/page.next.svg;"
        ;

WhiteCanvasTools::WhiteCanvasTools(QObject* parent, WhiteCanvas* whiteCanvas)
    : ToolButtonProvider(parent)
    , canvas_(nullptr)
    , pageList_(nullptr)
{
    setToolsString(toolstr);
    followTrigger();
    if (whiteCanvas)
        attachToWhiteCanvas(whiteCanvas);
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
        //QModelIndex index(canvas_->package()->currentModelIndex());
        //qobject_cast<QListView*>(pageList_)->scrollTo(index, QListView::PositionAtCenter);
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
    buttons[2]->setIconText(QString("%1/%2").arg(index + 1).arg(total));
    buttons[3]->setEnabled(index + 1 < total);
}

class ResourceImageProvider : public QQuickImageProvider
{
public:
    ResourceImageProvider(ResourcePackage * package)
        : QQuickImageProvider(Pixmap)
        , package_(package)
    {
    }
    // QQuickImageProvider interface
public:
    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &)
    {
        QPixmap pixmap = package_->pages().at(static_cast<int>(id.toFloat()))->thumbnail();
        *size = pixmap.size();
        return pixmap;
    }
private:
    ResourcePackage * package_;
};

QWidget *WhiteCanvasTools::createPageList(ResourcePackage * package)
{
    QQuickWidget* widget = new QQuickWidget;
    widget->engine()->addImageProvider("resource", new ResourceImageProvider(package));
    widget->setClearColor(Qt::transparent);
    widget->rootContext()->setContextProperty("packageModel", package);
    widget->setSource(QUrl("qrc:/showboard/qml/PageList.qml"));
    widget->installEventFilter(this);
    return widget;
}

bool WhiteCanvasTools::eventFilter(QObject * watched, QEvent *event)
{
    QWidget* widget = qobject_cast<QWidget*>(watched);
    if (event->type() == QEvent::Show) {
        widget->setFocus();
    }
    if (event->type() == QEvent::FocusOut) {
        widget->hide();
    }
    return false;
}
