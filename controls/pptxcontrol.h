#ifndef PPTXCONTROL_H
#define PPTXCONTROL_H

#include "core/control.h"

#include <QUrl>

class ResourceView;
class QAxObject;

class PptxControl : public Control
{
    Q_OBJECT

public:
    Q_INVOKABLE PptxControl(ResourceView * res);

    virtual ~PptxControl() override;

public:
    intptr_t hwnd() const;

signals:
    void opened();
    void closed();

public slots:
    void open();
    void show(int page = 0); // 0 for current page, 1 for first page
    void next();
    void prev();
    void jump(int page);
    void hide();
    void close();

private slots:
    void onPropertyChanged(const QString &name);

    void onSignal(const QString &name, int argc, void *argv);

    void onException(int code, const QString &source, const QString &desc, const QString &help);

protected:
    virtual QGraphicsItem * create(ResourceView * res) override;

    virtual QString toolsString() const override;

    virtual void detached() override;

private:
    void open(QUrl const & url);

    void reopen();

    void thumb(int page);

private:
    static QAxObject * application_;

    QString name_;
    int total_;
    int page_;

private:
    QAxObject * presentations_;
    QAxObject * presentation_;
    QAxObject * view_;
    intptr_t hwnd_;
};

#endif // PPTXCONTROL_H
