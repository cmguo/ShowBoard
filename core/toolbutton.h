#ifndef TOOLBUTTON_H
#define TOOLBUTTON_H

#include "ShowBoard_global.h"

#include <QVariant>

class SHOWBOARD_EXPORT ToolButton
{
    Q_GADGET

public:
    enum Flag
    {
        Popup = 1,
        Dynamic = 2, // need delete
        OptionsGroup = 4, // group of options
        Checkable = 8,
        NeedUpdate = 16,
        UnionUpdate = NeedUpdate | 32,
        CustomWidget = 64,
        Selected = 1 << 8,
        Checked = 1 << 9,
        HideSelector = 1 << 16,
    };

    Q_ENUM(Flag)

    Q_DECLARE_FLAGS(Flags, Flag)

    static ToolButton SPLITER;
    static ToolButton LINE_BREAK;

    static constexpr char const * ACTION_PROPERTY = "toolaction";

    typedef std::function<void()> action_t;

public:
    QString name;
    QString title;
    Flags flags;
    QVariant icon;

    static Flags makeFlags(const QString &str);

    static ToolButton * makeButton(QString const & desc);

    static QList<ToolButton *> makeButtons(QString const & tools);
};

Q_DECLARE_METATYPE(ToolButton)
Q_DECLARE_METATYPE(ToolButton::action_t)

#endif // TOOLBUTTON_H
