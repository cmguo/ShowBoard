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
        Dynamic = 1, // need delete
        Static = 2,
        OptionsGroup = 4, // group of options
        CustomWidget = 8,

        Popup = 16,
        Checkable = 32,
        NeedUpdate = 64,
        UnionUpdate = NeedUpdate | 128,
        // state
        Selected = 1 << 8,
        Checked = 1 << 9,
        Disabled = 1 << 10,
        HideSelector = 1 << 16,
    };

    Q_ENUM(Flag)

    Q_DECLARE_FLAGS(Flags, Flag)

    static ToolButton SPLITER;
    static ToolButton LINE_BREAK;
    static ToolButton PLACE_HOOLDER;

    static constexpr char const * ACTION_PROPERTY = "toolaction";

    typedef std::function<void()> action_t;

public:
    QByteArray name;
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
