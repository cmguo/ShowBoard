#include "wordscontrol.h"
#include "core/resourceview.h"
#include "core/optiontoolbuttons.h"

#include <QGraphicsPathItem>
#include <QPainter>
#include <QPen>
#include <QUrl>
#include <QDebug>

static constexpr char const * toolstr =
        "next()|下一笔|;"
        "fontWeight|粗细|Popup,OptionsGroup,NeedUpdate|;";

static EnumToolButtons weightButtons(QMetaEnum::fromType<QFont::Weight>());
REGISTER_OPTION_BUTTONS(WordsControl, fontWeight, weightButtons)

WordsControl::WordsControl(ResourceView *res)
    : Control(res)
{
    setToolsString(toolstr);
}

class WordsItem : public QGraphicsItem
{
public:
    WordsItem(QGraphicsItem * parent = nullptr);
    void setWords(QString const & words);
    void next();
public:
    virtual QRectF boundingRect() const override;
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override;
    QFont font_;
private:
    void addPath(QPainterPath & ph);
private:
    QPainterPath path_;
    QList<QPainterPath> paths_;
    QImage skeletons_;
    QPoint offset_;
    QList<int> lengths_;
    int step_ = 0;
};

ControlView *WordsControl::create(ControlView *)
{
    WordsItem * item = new WordsItem;
    item->setWords(res_->url().path());
    return item;
}

void WordsControl::next()
{
    WordsItem * item = static_cast<WordsItem*>(item_);
    item->next();
}

int WordsControl::fontWeight() const
{
    WordsItem * item = static_cast<WordsItem*>(item_);
    return item->font_.weight();
}

void WordsControl::setFontWeight(int weight)
{
    WordsItem * item = static_cast<WordsItem*>(item_);
    item->font_.setWeight(weight);
    item->setWords(res_->url().path());
}

/* WordsItem */

WordsItem::WordsItem(QGraphicsItem *parent)
    : QGraphicsItem(parent)
    , font_("KaiTi", 200)
{
}

static void distanceMap(QImage & image);

void WordsItem::setWords(QString const & words)
{
    prepareGeometryChange();
    path_ = QPainterPath();
    path_.addText({0, 0}, font_, words);
    path_.translate(-path_.boundingRect().center());
    QPainterPath p;
    paths_.clear();
    for (int i = 0; i < path_.elementCount(); ++i) {
        QPainterPath::Element e = path_.elementAt(i);
        switch (e.type) {
        case QPainterPath::MoveToElement:
            if (!p.isEmpty())
                addPath(p);
            p = QPainterPath(QPointF(e.x, e.y));
            break;
        case QPainterPath::LineToElement:
            p.lineTo(e.x, e.y);
            break;
        case QPainterPath::CurveToElement:
            if (i + 2 < path_.elementCount()) {
                QPainterPath::Element e1 = path_.elementAt(++i);
                QPainterPath::Element e2 = path_.elementAt(++i);
                p.cubicTo(QPointF(e.x, e.y), QPointF(e1.x, e1.y), QPointF(e2.x, e2.y));
            }
            break;
        default:
            break;
        }
    }
    if (!p.isEmpty())
        addPath(p);
    QRect rect = path_.boundingRect().toAlignedRect().adjusted(-1, -1, 1, 1);
    QImage image(rect.size(), QImage::Format_ARGB32);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    painter.setBrush(Qt::white);
    painter.setTransform(QTransform::fromTranslate(-rect.left(), -rect.top()));
    painter.drawPath(path_);
    painter.end();
    distanceMap(image);
    skeletons_ = image;
    offset_ = rect.topLeft();
}

void WordsItem::next()
{
    ++step_;
    update();
}

QRectF WordsItem::boundingRect() const
{
    return path_.boundingRect();
}

void WordsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
//    int l = 0;
//    for (int i = 0; i < paths_.size(); ++i) {
//        if (l + lengths_[i] <= step_) {
//            painter->setBrush(Qt::red);
//            painter->drawPath(paths_[i]);
//        //} else if (l < step_) {

//        } else {
//            painter->setBrush(Qt::white);
//            painter->drawPath(paths_[i]);
//        }
//        l += lengths_[i];
//    }
    painter->drawImage(offset_, skeletons_);
}

void WordsItem::addPath(QPainterPath &ph)
{
    ph.closeSubpath();
    for (QPainterPath & p : paths_) {
        if (p.intersects(ph)) {
            p.addPath(ph);
            return;
        }
    }
    paths_.append(ph);
    lengths_.append(1);
}

void distanceMap(QImage & image)
{
    constexpr int D = 16;
    QList<QPoint> points;
    int c1 = 1;
    int c2 = 1;
    int c3 = 1;
    QRgb zero = qRgba(0, 0, 0, 0);
    QRgb full = qRgb(255, 255, 255);
    for (int x = 1; x < image.width() - 1; ++x) {
        if (image.pixel(x - 1, 1) == zero)
            ++c1;
        if (image.pixel(x, 1) == zero)
            ++c2;
        if (image.pixel(x + 1, 1) == zero)
            ++c3;
        for (int y = 1; y < image.height() - 1; ++y) {
            if (image.pixel(x - 1, y + 1) == zero)
                ++c1;
            if (image.pixel(x, y + 1) == zero)
                ++c2;
            if (image.pixel(x + 1, y + 1) == zero)
                ++c3;
            if (image.pixel(x, y) != zero) {
                if (c1 || c2 || c3) {
                    image.setPixel(x, y, qRgb(D, D, D));
                    points.append({x, y});
                } else {
                    image.setPixel(x, y, full);
                }
            }
            if (image.pixel(x - 1, y - 1) == zero)
                --c1;
            if (image.pixel(x, y - 1) == zero)
                --c2;
            if (image.pixel(x + 1, y - 1) == zero)
                --c3;
        }
        //qDebug() << c1 << c2 << c3;
        c1 = c2 = c3 = 1;
    }
    QList<QPoint> points2;
    while (!points.empty()) {
        QPoint p = points.takeFirst();
        QRgb c = image.pixel(p);
        QRgb c2 = c + qRgba(D, D, D, 0);
        int n = points.size();
        p += QPoint(-1, -1);
        for (int i = -1; i < 2; ++i) {
            for (int j = -1; j < 2; ++j) {
                QRgb c1 = image.pixel(p);
                if (c1 == full) {
                    image.setPixel(p, c2);
                    points.append(p);
                } else if (c1 == c) {
                    --n;
                }
                p.setY(p.y() + 1);
            }
            p += QPoint(1, -3);
        }
        if (n == points.size())
            points2.append(p + QPoint(-2, 1));
    }
    for (QPoint const & p : points2)
        image.setPixel(p, qRgb(255, 0, 0));
}

void skeletons(QImage & image)
{
    QList<QPoint> points;
    for (int x = 1; x < image.width() - 1; ++x) {
        for (int y = 1; y < image.height() - 1; ++y) {

        }
    }
}
