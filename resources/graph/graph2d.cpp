#include "graph2d.h"
#include "core/resource.h"

using namespace QtPromise;

Graph2D::Graph2D(Resource * res)
    : Graph(res)
{
}

Graph2D::Graph2D(QPointF const & pt)
    : Graph("graph2d")
{
    addPoint(pt);
}

Graph2D::Graph2D(Graph2D const & o)
    : Graph(o)
{
}

bool Graph2D::empty() const
{
    return Graph::empty() && res_->url().path().length() < 15;
}

bool Graph2D::commit(const QPointF &pt)
{
    (void) pt;
    if (points_.size() == 1) {
        movePoint(points_.first() + QPointF(80, 80));
    }
    return true;
}

QPainterPath Graph2D::path()
{
    QPainterPath ph(points_[0]);
    for (int i = 1; i < points_.size(); ++i)
    {
        ph.lineTo(points_[i]);
    }
    ph.closeSubpath();
    return ph;
}

QPromise<void> Graph2D::load()
{
    QWeakPointer<int> life(this->life());
    if (!Graph::empty())
        return QPromise<void>::resolve();
    return res_->getStream().then([this, life](QIODevice * s) {
        if (life.isNull())
            return;
        QDataStream ds(s);
        int n = 0;
        qreal x, y;
        ds >> n;
        while (n) {
            ds >> x >> y;
            addPoint(QPointF(x, y));
            ++n;
        }
    });
}

Graph2DFactory::Graph2DFactory()
{
}

ResourceView * Graph2DFactory::create(Resource *res)
{
    QString type = res->url().path().mid(1);
    return ResourceFactory::create(res, type);
}

QUrl Graph2DFactory::newUrl(const QString &type) const
{
    return "graph2d:///" + type;
}
