#include "toolbutton.h"

ToolButton ToolButton::SPLITER({"spliter", "", nullptr, QVariant()});

ToolButton::Flags ToolButton::makeFlags(const QString &str)
{
   QStringList tokens = str.split(",", QString::SkipEmptyParts);
   Flags flags;
   for (QString const & t : tokens) {
       flags |= QVariant(t).value<Flag>();
   }
   return flags;
}

ToolButton * ToolButton::makeButton(const QString &desc)
{
    if (desc.startsWith("-"))
        return nullptr;
    if (desc == "|")
        return &SPLITER;
    QStringList seps = desc.split("|");
    if (seps.size() >= 1) {
        return new ToolButton{
            seps[0],
            seps.size() > 1 ? seps[1] : seps[0],
            seps.size() > 3 ? ToolButton::makeFlags(seps[2]) : nullptr,
            seps.size() > 2 ? QVariant(seps.back()) : QVariant()
        };
    }
    return nullptr;
}

QList<ToolButton *> ToolButton::makeButtons(QString const & tools)
{
    QList<ToolButton *> list;
    QStringList descs = tools.split(";", QString::SkipEmptyParts);
    for (QString desc : descs) {
        ToolButton * btn = makeButton(desc);
        if (btn) {
            list.append(btn);
        }
    }
    return list;
}
