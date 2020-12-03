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
    Q_CLASSINFO("position", "right")

public:
    Q_INVOKABLE WhiteCanvasTools(QObject* parent = nullptr, WhiteCanvas* whiteCanvas = nullptr);

public:
    Q_INVOKABLE void attachToWhiteCanvas(WhiteCanvas* whiteCanvas);

protected slots:
    void newPage();

    void prevPage();

    void pageList();

    void nextPage();

    void gotoPage(int n);

    void delPage();

    virtual bool setOption(QByteArray const & key, QVariant value) override;

private:
    void update();

private:
    QWidget * createPageList(ResourcePackage * package);

private:
    WhiteCanvas * canvas_;
    QWidget* pageList_;
};

#endif // WHITECANVASTOOLS_H
