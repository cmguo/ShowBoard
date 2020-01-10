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
    QTextEdit* textEdit = new QTextEdit;
    textEdit->setReadOnly(true);
    textEdit->setTextColor(QColor(Qt::black));
    textEdit->setAttribute(Qt::WA_StyledBackground);
    textEdit->setStyleSheet(QString("QTextEdit{background-color:#FFFFFFFF; border: 1px solid #FFFFFF; font: %1pt \"Microsoft YaHei UI\";}")
                        .arg(24));
    textEdit->resize(1024, 600);
    return textEdit;
}

void TextControl::attached()
{
    loadText();
}

void TextControl::onText(QString text)
{
    QTextEdit* textEdit = qobject_cast<QTextEdit*>(widget_);
    textEdit->setText(text);
}

