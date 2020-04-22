#include "strokes.h"
#include "strokeparser.h"
#include "core/resource.h"

using namespace QtPromise;

Strokes::Strokes(Resource * res, Flags flags, Flags clearFlags)
    : ResourceView(res, flags | TopMost, clearFlags | CanDelete)
    , scale_(1.0)
{
}

Strokes::Strokes(Strokes const & o)
    : ResourceView(o)
    , maximun_(o.maximun_)
    , points_(o.points_)
    , scale_(o.scale_)
{
}

QPromise<void> Strokes::load(QSizeF const & maxSize, IDynamicRenderer * dr)
{
    stream_.reset();
    if (url().scheme() == res_->type())
        return QPromise<void>::resolve();
    int n = url().path().lastIndexOf('.');
    if (n < 0)
        throw std::invalid_argument("no stroke type");
    qDebug() << "Strokes::load";
    QByteArray type = url().path().mid(n + 1).toUtf8();
    QWeakPointer<int> life(this->life());
    return res_->getStream().then([this, life, dr, type, maxSize](QSharedPointer<QIODevice> stream) {
        if (life.isNull())
            return;
        StrokeParser * sp = StrokeParser::instance();
        if (!sp)
            throw std::runtime_error("StrokeParser not found");
        maximun_ = sp->load(type, stream.get(), points_, dr);
        qDebug() << "Strokes::loaded";
        if (canvasSize().isEmpty())
            throw std::invalid_argument("empty canvas size");
        qreal scale = 1.0;
        QSizeF size = canvasSize();
        scale = qMin(maxSize.width() / size.width(), maxSize.height() / size.height());
        scale_ = scale;
        if (dr)
            stream_ = stream;
    });
}
