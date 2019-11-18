#include "toolbutton.h"

ToolButton::Flags ToolButton::makeFlags(const QString &str)
{
   QStringList tokens = str.split(",", QString::SkipEmptyParts);
   Flags flags;
   for (QString const & t : tokens) {
       flags |= QVariant(t).value<Flag>();
   }
   return flags;
}
