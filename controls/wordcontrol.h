#ifndef WORDCONTROL_H
#define WORDCONTROL_H

#include "core/control.h"

#include <QUrl>

class ResourceView;
class Word;

class WordControl : public Control
{
    Q_OBJECT

    Q_PROPERTY(int page READ page WRITE setPage)

public:
    Q_INVOKABLE WordControl(ResourceView * res);

    virtual ~WordControl() override;

public:
    int page();

    void setPage(int n);

public slots:
    void open();
    void show(int page = 0); // 0 for current page, 1 for first page
    void next();
    void prev();
    void jump(int page);
    void hide();
    void close();

private:
    void opened(int total);
    void reopened();
    void thumbed(QPixmap pixmap);
    void showed();
    void closed();
    void failed(QString const & msg);

protected:
    virtual QGraphicsItem * create(ResourceView * res) override;

    virtual QString toolsString(QString const & parent) const override;

    virtual void attaching() override;

    virtual void attached() override;

    virtual void detached() override;

private:
    void open(QUrl const & url);

private:
    Word * word_;
};

#endif // WORDCONTROL_H
