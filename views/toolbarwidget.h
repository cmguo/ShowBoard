#ifndef TOOLBARWIDGET_H
#define TOOLBARWIDGET_H

#include "ShowBoard_global.h"

#include "core/toolbutton.h"

#include <QFrame>
#include <QMap>
#include <QList>
#include <QPen>

class QLayout;
class ToolButtonProvider;
class QPushButton;
class QssHelper;
class QGraphicsItem;

class SHOWBOARD_EXPORT QFrameEx : public QFrame
{
public:
    QFrameEx(QWidget *parent = nullptr);

    void setStyleSheet(QssHelper const & style);

private:
    virtual void paintEvent(QPaintEvent *) override;

    virtual void connectNotify(const QMetaMethod &signal) override;

private:
    int borderRadius_;
    QPen borderPen_;
};

class SHOWBOARD_EXPORT ToolbarWidget : public QFrameEx
{
    Q_OBJECT
public:
    enum PopupPosition
    {
        TopLeft,
        TopCenter,
        TopRight,
        BottomLeft,
        BottomCenter,
        BottomRight,
    };
public:
    explicit ToolbarWidget(QWidget *parent = nullptr);

    explicit ToolbarWidget(bool horizontal, QWidget *parent = nullptr);

    virtual ~ToolbarWidget() override;

public:
    void setButtonTemplate(int typeId);

    void setStyleSheet(QssHelper const & style);

    void setDragable();

    void setPopupPosition(PopupPosition pos);

    QWidget * createPopup(QList<ToolButton *> const & buttons);

    QGraphicsItem* toGraphicsProxy(QGraphicsItem * parent = nullptr, bool centerPos = false);

public:
    void setToolButtons(QList<ToolButton *> const & buttons);

    void showPopupButtons(QList<ToolButton *> const & buttons);

    void updateButton(ToolButton * button);

    void updateToolButtons();

    void clear();

    void clearPopup();

public:
    void attachProvider(ToolButtonProvider* provider);

signals:
    void buttonClicked(QList<ToolButton *> const & buttons);

    void popupButtonsRequired(QList<ToolButton *> & buttons, QList<ToolButton *> const & parents);

    void sizeChanged(QSizeF const & size);

public slots:
    void buttonClicked();

public:
    virtual bool event(QEvent * event) override;

protected:
    virtual void getPopupButtons(QList<ToolButton *> & buttons,
                                QList<ToolButton *> const & parents);

    virtual void onButtonClicked(ToolButton * button);

    virtual QWidget* createPopupWidget();

private:
    virtual void setVisible(bool visible) override;

    virtual void mousePressEvent(QMouseEvent *event) override;

    virtual void mouseMoveEvent(QMouseEvent *event) override;

    virtual void mouseReleaseEvent(QMouseEvent *event) override;

    virtual void resizeEvent(QResizeEvent *event) override;

private:
    void addToolButton(QLayout * layout, ToolButton * button, QMap<QWidget *, ToolButton *>& buttons);

    static void applyButton(QPushButton * btn, ToolButton * button);

    void updateButton(QPushButton * btn, ToolButton * button);

    void setButtons(QLayout * layout, QList<ToolButton *> const & buttons, QMap<QWidget *, ToolButton *>& map);

    void clearButtons(QLayout * layout, QMap<QWidget *, ToolButton *>& buttons);

    void createPopup();

    void updateProvider();

    void buttonClicked(QWidget * btn);

    QPointF popupPosition(QPushButton * btn, QGraphicsProxyWidget * popup);

private:
    QMetaObject const * template_;
    QSize iconSize_;
    bool dragable_;
    //
    QLayout * layout_;
    QMap<QWidget *, ToolButton *> buttons_;
    //
    PopupPosition popupPosition_;
    QWidget * popUp_ = nullptr;
    QMap<QWidget *, ToolButton *> popupButtons_;
    QList<ToolButton *> popupParents_;
    QList<QWidget *> popupSplitWidget_;
    //
    ToolButtonProvider * provider_;
};

#endif // TOOLBARWIDGET_H
