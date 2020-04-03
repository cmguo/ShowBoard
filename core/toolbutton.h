#ifndef TOOLBUTTON_H
#define TOOLBUTTON_H

#include "ShowBoard_global.h"

#include <QAction>
#include <QVariant>

class SHOWBOARD_EXPORT ToolButton : public QAction
{
    Q_OBJECT

    Q_PROPERTY(QByteArray name READ name WRITE setName)
    Q_PROPERTY(bool dynamic READ isDynamic WRITE setDynamic) // need delete
    Q_PROPERTY(bool static READ isStatic WRITE setStatic)
    Q_PROPERTY(bool optionsGroup READ isOptionsGroup WRITE setOptionsGroup) // group of options
    Q_PROPERTY(bool customWidget READ isCustomWidget WRITE setCustomWidget)
    Q_PROPERTY(bool popup READ isPopup WRITE setPopup)
    Q_PROPERTY(bool hideSelector READ isHideSelector WRITE setHideSelector)
    Q_PROPERTY(bool needUpdate READ needUpdate WRITE setNeedUpdate)
    Q_PROPERTY(bool unionUpdate READ unionUpdate WRITE setUnionUpdate)
    Q_PROPERTY(QRectF itemRect READ itemRect WRITE setItemRect)

    Q_PROPERTY(QString iconSource READ iconSource NOTIFY changed)

public:
    static ToolButton SPLITTER;
    static ToolButton LINE_BREAK;
    static ToolButton LINE_SPLITTER;
    static ToolButton PLACE_HOOLDER;

    static constexpr char const * ACTION_PROPERTY = "toolaction";

    typedef std::function<void()> action_t;

public:
    enum Flag
    {
        Dynamic = 1, // need delete
        Static = 2,
        OptionsGroup = 4, // group of options
        CustomWidget = 8,

        Popup = 16,
        Checkable = 32,
        NeedUpdate = 64,
        UnionUpdate = NeedUpdate | 128,
        HideSelector = 1 << 8,
    };

    Q_DECLARE_FLAGS(Flags, Flag)

public:
    static QIcon makeIcon(QString const & iconString, QSize const & size);

    static QIcon makeIcon(QVariant& icon, QSize const & size, bool replace);

    static ToolButton * makeButton(QString const & desc);

    static QList<ToolButton *> makeButtons(QString const & tools);

public:
    ToolButton();

    ToolButton(ToolButton const & o);

    ToolButton(QByteArray const & name, QString const & title,
               Flags flags, QVariant const & icon = QVariant());

    ToolButton(QByteArray const & name, QString const & title,
               QByteArray const & flags, QVariant const & icon = QVariant());

signals:
    void delayActive(bool done);

public:
    QByteArray name() { return name_; }

    void setName(QByteArray const & name) { name_ = name; }

    bool isDynamic() const { return flags_.testFlag(Dynamic); }

    bool isStatic() const { return flags_.testFlag(Static); }

    bool isOptionsGroup() { return flags_.testFlag(OptionsGroup); }

    bool isCustomWidget() { return flags_.testFlag(CustomWidget); }

    bool isPopup() { return flags_.testFlag(Popup); }

    bool isHideSelector() { return flags_.testFlag(HideSelector); }

    bool needUpdate() { return flags_.testFlag(NeedUpdate); }

    bool unionUpdate() { return flags_.testFlag(UnionUpdate); }

    void setDynamic(bool v) { flags_.setFlag(Dynamic, v); }

    void setStatic(bool v) { flags_.setFlag(Static, v); }

    void setOptionsGroup(bool v) { flags_.setFlag(OptionsGroup, v); }

    void setCustomWidget(bool v) { flags_.setFlag(CustomWidget, v); }

    void setPopup(bool v) { flags_.setFlag(Popup, v); }

    void setHideSelector(bool v) { flags_.setFlag(HideSelector, v); }

    void setNeedUpdate(bool v) { flags_.setFlag(NeedUpdate, v); }

    void setUnionUpdate(bool v) { flags_.setFlag(UnionUpdate, v); }

    QRectF itemRect() const;

    void setItemRect(QRectF const & rect);

public:
    ToolButton(QByteArray const & name, Flags flags);

    void parseFlags(QByteArray const & flags);

    QIcon getIcon(QSize const & size = QSize());

    void setIcon(QVariant const & icon);

    QString iconSource();

    QWidget* getCustomWidget();

private:
    QByteArray name_;
    Flags flags_;
    QVariant icon_;
};

Q_DECLARE_METATYPE(ToolButton::action_t)

#endif // TOOLBUTTON_H
