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
        NameAsArgument = 4,
        HideSelector = 8,
    };

    Q_ENUM(Flag)

    Q_DECLARE_FLAGS(Flags, Flag)

public:
    QString name;
    QString title;
    Flags flags;
    QVariant icon;

    static Flags makeFlags(QString const & str);
};

Q_DECLARE_METATYPE(ToolButton)

#endif // TOOLBUTTON_H
