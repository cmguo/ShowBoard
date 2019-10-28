#ifndef PPTXCONTROL_H
#define PPTXCONTROL_H

#include "control.h"

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
    void open(int page = 0);
    void next();
    void prev();
    void jump(int page);
    void close();

private slots:
    void onPropertyChanged(const QString &name);

    void onSignal(const QString &name, int argc, void *argv);

    void onException(int code, const QString &source, const QString &desc, const QString &help);

protected:
    virtual QGraphicsItem * create(ResourceView * res) override;

    virtual QString toolsString() const override;

    virtual void detach() override;

private:
    void open_();

private:
    static QAxObject * application_;

    QUrl localUrl_;
    QString name_;
    QAxObject * presentation_;
    QAxObject * view_;
    intptr_t hwnd_;
    int startIndex_;
};

#endif // PPTXCONTROL_H
