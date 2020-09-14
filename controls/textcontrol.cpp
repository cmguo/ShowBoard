#include "textcontrol.h"
#include "core/resource.h"
#include "core/resourceview.h"

#include <QTextEdit>
#include <QMouseEvent>
#include <QScrollBar>
#include <QMimeData>

TextControl::TextControl(ResourceView * res)
    : WidgetControl(res, {WithSelectBar, ExpandScale, LayoutScale, FixedOnCanvas})
{
    setMinSize({0.1, 0.1});
}

QWidget *TextControl::createWidget(ResourceView *res)
{
    (void) res;
    QTextEdit* textEdit = new QTextEdit;
    textEdit->setReadOnly(true);
    textEdit->setTextInteractionFlags(Qt::NoTextInteraction);
    textEdit->setTextColor(QColor(Qt::black));
    textEdit->setAttribute(Qt::WA_StyledBackground);
    textEdit->setStyleSheet(QString("QTextEdit{background-color:#FFFFFFFF; border: 1px solid #FFFFFF; font: %1pt \"Microsoft YaHei UI\";}")
                        .arg(24));
    textEdit->resize(1024, 600);
    textEdit->viewport()->setMouseTracking(false);
    textEdit->viewport()->installEventFilter(this);
    return textEdit;
}

void TextControl::attached()
{
    if (auto data = res_->mimeData()) {
        onText(data->text());
        loadFinished(true);
        return;
    }
    loadText();
}

void TextControl::onText(QString text)
{
    QTextEdit* textEdit = qobject_cast<QTextEdit*>(widget_);
    textEdit->setText(text);
}

void TextControl::copy(QMimeData &data)
{
    Control::copy(data);
    QTextEdit* textEdit = qobject_cast<QTextEdit*>(widget_);
    data.setText(textEdit->toPlainText());
}

bool TextControl::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent* me = static_cast<QMouseEvent*>(event);
        lastPos_ = me->pos();
    } else if (event->type() == QEvent::MouseMove) {
        QMouseEvent* me = static_cast<QMouseEvent*>(event);
        QPoint d = lastPos_ - me->pos();
        lastPos_ = me->pos();
        QTextEdit* textEdit = qobject_cast<QTextEdit*>(widget_);
        textEdit->horizontalScrollBar()->setValue(textEdit->horizontalScrollBar()->value() + d.x());
        textEdit->verticalScrollBar()->setValue(textEdit->verticalScrollBar()->value() + d.y());
    } else {
        return WidgetControl::eventFilter(watched, event);
    }
    return true;
}

