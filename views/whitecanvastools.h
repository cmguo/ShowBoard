#ifndef WHITECANVASTOOLS_H
#define WHITECANVASTOOLS_H

#include <core/toolbuttonprovider.h>

class WhiteCanvas;

class WhiteCanvasTools : public ToolButtonProvider
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

private:
    WhiteCanvas * canvas_;
};

#endif // WHITECANVASTOOLS_H
