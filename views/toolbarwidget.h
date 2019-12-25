#ifndef TOOLBARWIDGET_H
#define TOOLBARWIDGET_H

#include "ShowBoard_global.h"

#include "core/toolbutton.h"

#include <QWidget>
#include <QMap>
#include <QList>

class QLayout;
class ToolButtonProvider;
class QPushButton;

class SHOWBOARD_EXPORT ToolbarWidget : public QWidget
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

public:
    void setButtonTemplate(int typeId);

    void setPopupPosition(PopupPosition pos);

public:
    void setToolButtons(QList<ToolButton *> const & buttons);

    void setToolButtons(ToolButton buttons[], int count);

    void showPopupButtons(QList<ToolButton *> const & buttons);

    void showPopupButtons(ToolButton buttons[], int count);

    void updateButton(ToolButton * button);

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

protected:
    virtual void getPopupButtons(QList<ToolButton *> & buttons,
                                QList<ToolButton *> const & parents);

    virtual void onButtonClicked(ToolButton * button);

private:
    virtual void resizeEvent(QResizeEvent *event) override;

    virtual void setVisible(bool visible) override;

private:
    void addToolButton(QLayout * layout, ToolButton * button, QMap<QWidget *, ToolButton *>& buttons);

    void applyButton(QPushButton * btn, ToolButton * parent, ToolButton * button);

    void updateButton(QPushButton * btn, ToolButton * parent, ToolButton * button);

    void clearButtons(QLayout * layout, QMap<QWidget *, ToolButton *>& buttons);

    void createPopup();

    void updateProvider();

    QPointF popupPosition(QPushButton * btn, QGraphicsProxyWidget * popup);

private:
    QMetaObject const * template_;
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
