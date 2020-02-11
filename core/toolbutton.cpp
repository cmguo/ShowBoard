#include "toolbutton.h"

ToolButton ToolButton::SPLITTER({"splitter", nullptr, Static, QVariant()});
ToolButton ToolButton::LINE_BREAK({"lineBreak", nullptr, Static, QVariant()});
ToolButton ToolButton::LINE_SPLITTER({"lineSplitter", nullptr, Static, QVariant()});
ToolButton ToolButton::PLACE_HOOLDER({"placeHolder", nullptr, Static, QVariant()});

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
    if (desc == "|")
        return &SPLITTER;
    if (desc == "")
        return &PLACE_HOOLDER;
    if (desc == "-")
        return &LINE_SPLITTER;
    if (desc.startsWith("-"))
        return nullptr;
    QStringList seps = desc.split("|");
    if (seps.size() >= 1) {
        return new ToolButton{
            seps[0].toUtf8(),
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
