#ifndef WHITECANVASTOOLS_H
#define WHITECANVASTOOLS_H

#include "ShowBoard_global.h"

#include <core/toolbuttonprovider.h>

#include <QVariant>

class WhiteCanvas;
class ResourcePackage;

class SHOWBOARD_EXPORT WhiteCanvasTools : public ToolButtonProvider
{
    Q_OBJECT
public:
    WhiteCanvasTools(QObject* parent = nullptr, WhiteCanvas* whiteCanvas = nullptr);

public:
    void attachToWhiteCanvas(WhiteCanvas* whiteCanvas);

protected slots:
    void newPage();

    void prevPage();

    void pageList();

    void nextPage();

    void gotoPage(int n);

    virtual void setOption(QByteArray const & key, QVariant value) override;

protected:
    virtual bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void update();

private:
    QWidget * createPageList(ResourcePackage * package);

private:
    WhiteCanvas * canvas_;
    QWidget* pageList_;
};

#endif // WHITECANVASTOOLS_H
