#ifndef RESOURCERECORD_H
#define RESOURCERECORD_H

#include "ShowBoard_global.h"
#include "controlview.h"

#include <QList>
#include <QObject>

class SHOWBOARD_EXPORT ResourceRecord
{
public:
    ResourceRecord() {}

    virtual ~ResourceRecord() {}

public:
    virtual void undo() = 0;

    virtual void redo() = 0;

    virtual void dump();

public:
    void setInfo(char const * info) { info_ = info; }

private:
    Q_DISABLE_COPY(ResourceRecord)
    char const * info_ = nullptr;
};

template <typename U, typename R>
class FunctionRecord : public ResourceRecord
{
public:
    FunctionRecord(U u, R r)
        : u_(u)
        , r_(r)
    {
    }
public:
    virtual void undo() override { u_(); }
    virtual void redo() override { r_(); }
private:
    U u_;
    R r_;
};

template <typename U, typename R>
inline FunctionRecord<U, R> * makeFunctionRecord(U u, R r)
{
    return new FunctionRecord<U, R>(u, r);
}

template <typename D>
class DestructRecord : public ResourceRecord
{
public:
    DestructRecord(bool destroy, D d) : destroy_(destroy), d_(d) { }
    virtual ~DestructRecord() override { if (destroy_) d_(); }
public:
    virtual void undo() override { destroy_ = !destroy_; }
    virtual void redo() override { destroy_ = !destroy_; }
private:
    bool destroy_ = false;
    D d_;
};

template <typename D>
inline DestructRecord<D> * makeDestructRecord(bool undo, D d)
{
    return new DestructRecord<D>(undo, d);
}

inline ResourceRecord * setRecordInfo(ResourceRecord * record, char const * info)
{
    if (record)
        record->setInfo(info);
    return record;
}
#define SHOWBOARD_TOSTRING2(x) #x
#define SHOWBOARD_TOSTRING(x) SHOWBOARD_TOSTRING2(x)
#ifdef __MACH__
#define SHOWBOARD_CODE_INFO __FILE__ "(" SHOWBOARD_TOSTRING(__LINE__) ")"
#else
#define SHOWBOARD_CODE_INFO __FUNCTION__ ": " __FILE__ "(" SHOWBOARD_TOSTRING(__LINE__) ")"
#endif
#define MakeFunctionRecord(...) \
    setRecordInfo(makeFunctionRecord(__VA_ARGS__), SHOWBOARD_CODE_INFO)
#define MakeDestructRecord(destroy, ...) \
    setRecordInfo(makeDestructRecord(destroy, __VA_ARGS__), SHOWBOARD_CODE_INFO)

class MergeRecord : public ResourceRecord
{
public:
    MergeRecord(QList<ResourceRecord*> const & records = {});

    virtual ~MergeRecord() override;

public:
    void add(ResourceRecord* record);

public:
    virtual void undo() override;

    virtual void redo() override;

    virtual void dump() override;

private:
    QList<ResourceRecord*> records_;
};

class SHOWBOARD_EXPORT ResourceRecordSet : public QObject
{
public:
    ResourceRecordSet(QObject * parent = nullptr, int capacity = 100);

    virtual ~ResourceRecordSet() override;

    static ResourceRecord * merge(ResourceRecord * dest, MergeRecord *& merge, ResourceRecord * record);

public:
    void commit(ResourceRecord * record);

    void undo();

    void redo();

    bool inOperation() const;

    void clear(int keep = 0);

public:
    /* merge operations */

    void prepare(bool blocked);

    int prepareLevel() const { return prepare_; }

    // if blocked, drop more deep records
    bool blocked() const { return block_ >= 0 && prepare_ > block_; }

    void add(ResourceRecord * record);

    void commit(bool drop = false);

private:
    QList<ResourceRecord*> records_;
    int capacity_;
    int undo_ = 0;
    ResourceRecord * operation_ = nullptr;

private:
    /* merge states */
    int prepare_ = 0;
    bool drop_ = false;
    int block_ = -1;
    ResourceRecord * record_ = nullptr;
    MergeRecord * mergeRecord_ = nullptr;
};

class ResourcePackage;
class ResourcePage;
class ResourceView;
class Control;

class SHOWBOARD_EXPORT RecordMergeScope
{
public:
    RecordMergeScope(ResourceRecordSet * set, bool block = false);
    RecordMergeScope(ResourcePackage * pkg, bool block = false);
    RecordMergeScope(ResourcePage * page, bool block = false);
    RecordMergeScope(ResourceView * res, bool block = false);
    RecordMergeScope(Control * ctrl, bool block = false);
    RecordMergeScope(ControlView * view, bool block = false);
    ~RecordMergeScope();
    operator bool() const;
    void add(ResourceRecord * record);
    void drop();
    bool atTop() const;
    bool inOperation() const;
private:
    Q_DISABLE_COPY(RecordMergeScope)
    ResourceRecordSet * set_;
    bool drop_ = false;
};

#endif // RESOURCERECORD_H
