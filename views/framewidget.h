#ifndef FRAMEWIDGET_H
#define FRAMEWIDGET_H

#include <QWidget>

class ResourceView;

class FrameWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FrameWidget(QWidget *parent = nullptr);

signals:

private:
    ResourceView* path_;
};

#endif // FRAMEWIDGET_H
