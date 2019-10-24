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

public slots:
    void show();
    void next();
    void prev();
    void exit();

private slots:
    void onPropertyChanged(const QString &name);

    void onSignal(const QString &name, int argc, void *argv);

    void onException(int code, const QString &source, const QString &desc, const QString &help);

protected:
    virtual QGraphicsItem * create(ResourceView * res) override;

    virtual QString toolsString() const override;

    virtual void detach() override;

private:
    void open();

private:
    static QAxObject * application_;

    QUrl localUrl_;
    QAxObject * presentation_;
    QAxObject * view_;
};

#endif // PPTXCONTROL_H
