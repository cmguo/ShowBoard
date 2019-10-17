#ifndef TOOLBARWIDGET_H
#define TOOLBARWIDGET_H

#include <QWidget>
#include <QMap>

class QHBoxLayout;

class ToolbarWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ToolbarWidget(QWidget *parent = nullptr);

public:
    typedef std::function<void(void)> Action;

    void addCommond(QString const & title, QString const & icon, Action action);

    void clear();

signals:

public slots:

private:
    QHBoxLayout * layout_;
    QList<QWidget *> buttons_;
};

#endif // TOOLBARWIDGET_H
