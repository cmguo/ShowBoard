#include "resourceview.h"
#include "resource.h"
#include "resourcetransform.h"
#include "resourcepage.h"
#include "resourcemanager.h"

#include <QApplication>
#include <QGraphicsItem>
#include <QMetaMethod>
#include <QMimeData>
#include <QVariant>

char const * const ResourceView::EXPORT_ATTR_FACTORY = "rfactory";

static constexpr char const * SESSION_GROUP = "SESSION_GROUP";
static constexpr char const * SESSION = "SESSION";
#if QT_VERSION >= 0x050E00
static constexpr int MAX_SESSION = 30;
#else
static constexpr int MAX_SESSION = 20;
#endif

ResourceView::ResourceView(Resource * res, Flags flags, Flags clearFlags)
    : res_(res)
    , flags_((DefaultFlags | flags) & ~clearFlags)
    , transform_(new ResourceTransform(this))
{
    QString path = res->url().path();
    if (path.size() > 1)
        name_ = path.mid(path.lastIndexOf('/') + 1);
    else
        name_ = res->url().scheme();
    res_->setParent(this);
}

ResourceView::ResourceView(QByteArray const & type, QUrl const & url)
    : ResourceView(new Resource(type, url), {}, CanCopy) // not copyable
{
}

ResourceView::ResourceView(ResourceView *mainRes)
    : ResourceView(new Resource("whitecanvas", QUrl("whitecanvas:///")),
                   BottomMost | mainRes->pageMode(),
                   ~mainRes->flags() & DefaultFlags)
{
    mainRes->flags_.setFlag(ResourceView::CanDelete, false);
    setOverrideToolString(mainRes->overrideToolString());
}

ResourceView::~ResourceView()
{
}

bool ResourceView::deletable() const
{
    return flags_.testFlag(CanDelete);
}

void ResourceView::setDeletable(bool v)
{
    flags_.setFlag(CanDelete, v);
}

bool ResourceView::copyable() const
{
    return flags_.testFlag(CanCopy);
}

void ResourceView::setCopyable(bool v)
{
    flags_.setFlag(CanCopy, v);
}

bool ResourceView::independent() const
{
    return flags_.testFlag(Independent);
}

void ResourceView::setIndependent(bool v)
{
    flags_.setFlag(Independent, v);
}

ResourceView::Flags ResourceView::pageMode() const
{
    return flags_ & (LargeCanvas | VirtualPage | ListOfPages);
}

void ResourceView::setPageMode(Flags mode)
{
    constexpr Flags modeFlags{LargeCanvas, VirtualPage, ListOfPages};
    flags_ = (flags_ & ~modeFlags) | (mode & modeFlags);
}

QByteArray ResourceView::sessionGroup()
{
    return res_->property(SESSION_GROUP).toByteArray();
}

void ResourceView::setSessionGroup(const QByteArray &session)
{
    flags_.setFlag(PersistSession);
    res_->setProperty(SESSION_GROUP, session);
}

class ResourceSession
{
public:
    ResourceSession() : view_(nullptr), item_(nullptr) {}
    ResourceSession(ResourceView * view, QGraphicsItem * item)
        : view_(view)
        , item_(item)
    {
    }
    ~ResourceSession()
    {
        delete detach();
    }
    void clear()
    {
        if (view_)
            view_->clearSession();
        // here this is detroyed
    }
    QGraphicsItem* detach()
    {
        QGraphicsItem * item = item_;
        view_ = nullptr;
        item_ = nullptr;
        return item;
    }
private:
    ResourceView * view_;
    QGraphicsItem * item_;
};

Q_DECLARE_METATYPE(QSharedPointer<ResourceSession>)

static QMap<QByteArray, QWeakPointer<ResourceSession>> groupSessions;
static QVector<QWeakPointer<ResourceSession>> allSessions;

static bool dropOneSession()
{
    if (allSessions.isEmpty())
        return false;
    QWeakPointer<ResourceSession> groupSession = allSessions.first();
    QSharedPointer<ResourceSession> groupSession2 = groupSession.toStrongRef();
    if (groupSession2) {
        groupSession2->clear();
        qDebug() << "ResourceView dropOneSession" << allSessions.size();
    }
    return true;
}

QGraphicsItem *ResourceView::loadSession()
{
    QSharedPointer<ResourceSession> session =
            res_->property(SESSION).value<QSharedPointer<ResourceSession>>();
    res_->setProperty(SESSION, QVariant());
    QGraphicsItem * item = session ? session->detach() : nullptr;
    QByteArray group = sessionGroup();
    if (item && !group.isEmpty()) {
        QWeakPointer<ResourceSession> groupSession = groupSessions.take(group);
        QSharedPointer<ResourceSession> groupSession2 = groupSession.toStrongRef();
        if (groupSession2)
            groupSession2->clear();
    }
    allSessions.removeOne(session);
    return item;
}

void ResourceView::saveSession(QGraphicsItem *item)
{
    static bool initOom = false;
    if (!initOom) {
        Resource::registerOutOfMemoryHandler(0, dropOneSession);
        initOom = true;
    }
    QSharedPointer<ResourceSession> session(new ResourceSession(this, item));
    res_->setProperty(SESSION, QVariant::fromValue(session));
    QByteArray group = sessionGroup();
    if (!group.isEmpty()) {
        QWeakPointer<ResourceSession> groupSession = groupSessions.take(group);
        QSharedPointer<ResourceSession> groupSession2 = groupSession.toStrongRef();
        if (groupSession2) {
            groupSession2->clear();
        }
        groupSessions.insert(group, session);
        allSessions.removeOne(groupSession);
    }
    allSessions.append(session);
    qDebug() << "ResourceView saveSession" << allSessions.size();
    if (allSessions.size() > MAX_SESSION) {
        dropOneSession();
    }
}

void ResourceView::clearSession()
{
    qWarning() << "ResourceView: clear session" << url();
    delete loadSession();
}

ResourceView::ResourceView(ResourceView const & o)
    : ToolButtonProvider(o)
    , res_(new Resource(*o.res_))
    , flags_(o.flags_)
    , name_(o.name_)
    , transform_(new ResourceTransform(*o.transform_, this))
{
    //flags_ &= ~SavedSession;
    res_->setParent(this);
    //transform_->translate({60, 60});
}

Q_DECLARE_METATYPE(QSharedPointer<QMimeData>)

QMimeData const * ResourceView::mimeData()
{
    return property("mimedata").value<QSharedPointer<QMimeData>>().get();
}

ResourceView * ResourceView::clone() const
{

    QObject * clone = metaObject()->newInstance(QGenericArgument(metaObject()->className(), this));
    if (clone)
        return qobject_cast<ResourceView*>(clone);
    return new ResourceView(*this);
}

void ResourceView::copy(QMimeData &data)
{
    data.setUrls({res_->url()});
    data.setText(res_->url().toString());
    data.setData("application/x-resource", QByteArray());
    data.setProperty("OriginPage", QVariant::fromValue(page()));
    ResourceView * res = clone();
    res->setParent(&data);
}

class CopyMimeData : public QMimeData
{
public:
    CopyMimeData(QMimeData const & data)
    {
        setImageData(data.imageData());
        setText(data.text());
        setHtml(data.html());
    }
};

ResourceView *ResourceView::paste(QMimeData const &data)
{
    if (data.hasFormat("application/x-resource")) {
        ResourceView * res = data.findChild<ResourceView*>(nullptr, Qt::FindDirectChildrenOnly);
        if (res) {
            ResourcePage * po = data.property("OriginPage").value<ResourcePage*>();
            ResourcePage * pt = data.property("TargetPage").value<ResourcePage*>();
            if (po == pt) {
                res->transform().translate({60, 60});
            } else {
                res->transform().translateTo({0, 0});
            }
            const_cast<QMimeData&>(data).setProperty("OriginPage", QVariant::fromValue(pt));
            return res->clone();
        }
    }
    // we like urls
    if (data.hasUrls()) {
        ResourceView * res = ResourceManager::instance()->createResource(data.urls().first());
        res->setProperty("name", "链接");
        return res;
    }
    for (auto f : data.formats()) {
        int n = f.indexOf('/');
        QUrl url = n > 0 ? QUrl(f.left(n) + ":mimedata." + f.mid(n + 1))
                         : QUrl(f.left(n) + ":");
        if (ResourceManager::instance()->isExplitSupported(url)) {
            ResourceView * res = ResourceManager::instance()->createResource(url);
            QSharedPointer<QMimeData> cd(new CopyMimeData(data));
            res->setProperty("mimedata", QVariant::fromValue(cd));
            return res;
        }
    }
    return nullptr;
}

QUrl const & ResourceView::url() const
{
    return res_->url();
}

void ResourceView::moveTop()
{
    if (flags_ & (ResourceView::ZOrderFlags))
        return;
    qobject_cast<ResourcePage*>(parent())->moveResourceBack(this);
}

bool ResourceView::canMoveTop()
{
    if (flags_ & (ResourceView::ZOrderFlags))
        return false;
    return qobject_cast<ResourcePage*>(parent())->nextNormalResource(this);
}

void ResourceView::removeFromPage()
{
    qobject_cast<ResourcePage*>(parent())->removeResource(this);
}

ResourcePage *ResourceView::page()
{
    return qobject_cast<ResourcePage*>(parent());
}

void ResourceView::setSaved()
{
    flags_ |= SavedSession;
}

