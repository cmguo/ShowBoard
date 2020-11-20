#ifndef DOCXCONTROL_H
#define DOCXCONTROL_H

#include "imagecontrol.h"

#include <QUrl>

class ResourceView;
class Word;

class DocxControl : public ImageControl
{
    Q_OBJECT

    Q_PROPERTY(int page READ page WRITE setPage)

public:
    Q_INVOKABLE DocxControl(ResourceView * res);

    virtual ~DocxControl() override;

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
    virtual QString toolsString(QByteArray const & parent) const override;

    virtual void attached() override;

    virtual void detached() override;

private:
    void open(QUrl const & url);

private:
    Word * word_;
};

#endif // DOCXCONTROL_H
