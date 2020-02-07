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
    void next();
    void prev();
    void jump(int page);
    void close();

private:
    void opened(int total);
    void thumbed(QPixmap pixmap);
    void closed();
    void failed(QString const & msg);

protected:
    virtual QGraphicsItem * create(ResourceView * res) override;

    virtual QString toolsString(QByteArray const & parent) const override;

    virtual void attaching() override;

    virtual void attached() override;

    virtual void detached() override;

private:
    void open(QUrl const & url);

private:
    Word * word_;
};

#endif // WORDCONTROL_H
