#ifndef WHITECANVASTOOLS_H
#define WHITECANVASTOOLS_H

#include "ShowBoard_global.h"

#include <core/toolbuttonprovider.h>

class WhiteCanvas;
class ResourcePackage;

class SHOWBOARD_EXPORT WhiteCanvasTools : public ToolButtonProvider
{
public:
    WhiteCanvasTools();

public:
    void attachToWhiteCanvas(WhiteCanvas* whiteCanvas);

protected slots:
    void newPage();

    void prevPage();

    void pageList();

    void nextPage();

private:
    void update();

public:
    static QWidget * createPageList(ResourcePackage * package);

private:
    WhiteCanvas * canvas_;
    QWidget* pageList_;
};

#endif // WHITECANVASTOOLS_H
