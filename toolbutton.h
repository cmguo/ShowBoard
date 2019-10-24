#ifndef TOOLBUTTON_H
#define TOOLBUTTON_H

#include <QVariant>

struct ToolButton
{
    QString name;
    QString title;
    QVariant icon;
};

Q_DECLARE_METATYPE(ToolButton)

#endif // TOOLBUTTON_H
