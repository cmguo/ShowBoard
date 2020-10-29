#include "resourceview.h"
#include "resource.h"
#include "resourcetransform.h"
#include "resourcepage.h"
#include "resourcemanager.h"

#ifdef SHOWBOARD_QUICK
#include <QQuickItem>
#else
#include <QGraphicsItem>
#endif
#include <QApplication>
#include <QMetaMethod>
#include <QMimeData>
#include <QVariant>

char const * const ResourceView::EXPORT_ATTR_FACTORY = "rfactory";

static constexpr char const * SESSION_GROUP = "SESSION_GROUP";
static constexpr char const * SESSION = "SESSION";
static constexpr int MAX_SESSION = sizeof(void*) == 4 ? 10 : 20;

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
    ResourceSession(ResourceView * view, ControlView * item)
        : view_(view)
        , item_(item)
    {
        priority_ = view->property("sessionPriority").toInt();
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
    int priority() const { return priority_; }
    ControlView* detach()
    {
        ControlView * item = item_;
        view_ = nullptr;
        item_ = nullptr;
        return item;
    }
private:
    ResourceView * view_;
    ControlView * item_;
    int priority_ = 0;
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
        qWarning() << "ResourceView dropOneSession" << allSessions.size();
    } else {
        allSessions.pop_front();
    }
    return true;
}

ControlView *ResourceView::loadSession()
{
    QSharedPointer<ResourceSession> session =
            res_->property(SESSION).value<QSharedPointer<ResourceSession>>();
    res_->setProperty(SESSION, QVariant());
    ControlView * item = session ? session->detach() : nullptr;
    QByteArray group = sessionGroup();
    if (item && !group.isEmpty()) {
        groupSessions.remove(group);
    }
    allSessions.removeOne(session);
    return item;
}

void ResourceView::saveSession(ControlView *item)
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
    auto iter = std::upper_bound(allSessions.begin(), allSessions.end(),
                                 session.toWeakRef(), [] (auto l , auto r) {
        if (l.isNull())
            return true;
        if (r.isNull())
            return false;
        return l.data()->priority() < r.data()->priority();
    });
    if (iter == allSessions.end())
        allSessions.append(session);
    else
        allSessions.insert(iter, session);
    qDebug() << "ResourceView saveSession" << allSessions.size();
#ifndef QT_DEBUG
    if (allSessions.size() > MAX_SESSION) {
        dropOneSession();
    }
#else
    (void) MAX_SESSION;
#endif
}

void ResourceView::clearSession()
{
    qWarning() << "ResourceView: clear session" << name();
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

static constexpr char const * RESOURCE_FORMAT = "application/x-resource";

void ResourceView::copy(QMimeData &data)
{
    data.setUrls({res_->url()});
    data.setText(res_->url().toString());
    data.setData(RESOURCE_FORMAT, QByteArray());
    ResourceView * res = clone();
    res->setParent(&data);
}

bool ResourceView::canPaste(const QMimeData &data)
{
    if (data.hasFormat(RESOURCE_FORMAT))
        return data.data(RESOURCE_FORMAT).isEmpty();
    ResourceView * res = paste(data);
    if (res) {
        const_cast<QMimeData&>(data).setData(RESOURCE_FORMAT, QByteArray());
        res->setParent(&const_cast<QMimeData&>(data));
        return true;
    } else {
        const_cast<QMimeData&>(data).setData(RESOURCE_FORMAT, "X");
        return false;
    }
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
    if (data.hasFormat(RESOURCE_FORMAT)) {
        ResourceView * res = data.findChild<ResourceView*>(
                    nullptr /* aName */, Qt::FindDirectChildrenOnly);
        if (res)
            return res->clone();
    }
    // we like urls
    if (data.hasUrls() && !data.urls().isEmpty()) {
        QUrl url = data.urls().first();
        ResourceView * res = ResourceManager::instance()->createResource(url);
        if (res) {
            res->setProperty("name", "链接");
            return res;
        }
    }
    if (data.hasText()) {
         QUrl url(data.text());
         if (url.isValid() && !url.scheme().isEmpty()) {
             ResourceView * res = ResourceManager::instance()->createResource(url);
             if (res) {
                 res->setProperty("name", "链接");
                return res;
             }
         }
    }
    for (auto f : data.formats()) {
#ifndef QT_DEBUG
        // Bugly: avoid unespect drag from web page
        if (f.startsWith("text/") && data.text().length() < 64)
            break;
#endif
        int n = f.indexOf('/');
        QUrl url = n > 0 ? QUrl(f.mid(n + 1) + ":mimedata." + f.left(n))
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

ResourcePackage *ResourceView::package()
{
    ResourcePage * p = page();
    return p ? p->package() : nullptr;
}

void ResourceView::setSaved()
{
    flags_ |= SavedSession;
}

