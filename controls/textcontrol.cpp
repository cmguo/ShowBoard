#include "textcontrol.h"
#include "core/resource.h"
#include "core/resourceview.h"

#include <QTextCodec>
#include <QTextEdit>

TextControl::TextControl(ResourceView * res)
    : WidgetControl(res, {WithSelectBar, ExpandScale, LayoutScale})
{
}

QWidget *TextControl::createWidget(ResourceView *res)
{
    (void) res;
    QTextEdit* text = new QTextEdit;
    text->setReadOnly(true);
    text->setTextColor(QColor(Qt::black));
    text->setStyleSheet(QString("QTextEdit{background-color:#FFFFFF; border: 1px solid #FFFFFF; font: %1pt \"Microsoft YaHei UI\";}")
                        .arg(24));
    return text;
}

void TextControl::attached()
{
    loadText();
}

void TextControl::onText(QString text)
{
    qobject_cast<QTextEdit*>(widget_)->setText(text);
}

