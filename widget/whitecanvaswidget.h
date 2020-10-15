#ifndef WHITECANVASWIDGET_H
#define WHITECANVASWIDGET_H

#include "ShowBoard_global.h"

#ifdef SHOWBOARD_QUICK
#include <QQuickWidget>
#else
#include <QGraphicsView>
#endif

class QGraphicsScene;
class QQuickWindow;
class WhiteCanvas;
class ResourcePackage;
class ResourcePage;
class Control;

#ifdef SHOWBOARD_QUICK
class SHOWBOARD_EXPORT WhiteCanvasWidget : public QQuickWidget
#else
class SHOWBOARD_EXPORT WhiteCanvasWidget : public QGraphicsView
#endif
{
    Q_OBJECT
public:
    explicit WhiteCanvasWidget(QWidget *parent = nullptr);

    explicit WhiteCanvasWidget(WhiteCanvasWidget* mainView, QWidget *parent = nullptr);

    virtual ~WhiteCanvasWidget() override;

    static WhiteCanvasWidget * mainInstance();

public:
    WhiteCanvas * canvas()
    {
        return canvas_;
    }

    ResourcePackage * package();

    void setSceneSize(QSizeF size);

    void setSceneBackgroundColor(QColor const & color);

    void setResourcePackage(ResourcePackage * pack);

private slots:
    // Delete
    void deleteSelection();

    // Escape
    void cancelSelection();

    // L/T/R/D
    void moveSelection();

    // Shift + L/T/R/D
    void scaleSelection();

    // Shift + +/-, scale without direction
    void scaleSelection2();

    // Control + L/T/R/D
    // PageUp, PageDown, Space, Backspace
    void switchPage();

    // Control + C/X/V
    void copyPaste();

    // Control + X/Y
    void undoRedo();

private:
    virtual void resizeEvent(QResizeEvent *event) override;

    virtual void showEvent(QShowEvent *event) override;

    virtual void dragEnterEvent(QDragEnterEvent *event) override;

    virtual void dragMoveEvent(QDragMoveEvent *event) override;

    virtual void dragLeaveEvent(QDragLeaveEvent *event) override;

    virtual void dropEvent(QDropEvent *event) override;

    virtual bool eventFilter(QObject *watched, QEvent *event) override;

    virtual void keyReleaseEvent(QKeyEvent *event) override;

private:
    void onPageChanged(ResourcePage* page);

    bool onShotcut(Control * control);

private:
#ifdef SHOWBOARD_QUICK
    QQuickItem * scene_;
#else
    QGraphicsScene * scene_;
#endif
    WhiteCanvas * canvas_;
    QSizeF sceneSize_;
    Control * shotcutControl_;
    bool dragAccept_;
};

#endif // WHITECANVASWIDGET_H
