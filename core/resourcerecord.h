#ifndef RESOURCERECORD_H
#define RESOURCERECORD_H

#include <QList>

class ResourceRecord
{
public:
    virtual ~ResourceRecord() {}

public:
    virtual void undo() = 0;

    virtual void redo() = 0;
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
    DestructRecord(D d) : d_(d) { }
    virtual ~DestructRecord() override { d_(undo_); }
public:
    virtual void undo() override { undo_ = true; }
    virtual void redo() override { undo_ = false; }
private:
    bool undo_ = false;
    D d_;
};

template <typename D>
inline DestructRecord<D> * makeDestructRecord(D d)
{
    return new DestructRecord<D>(d);
}

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

private:
    QList<ResourceRecord*> records_;
};

class ResourceRecordSet : public QObject
{
public:
    ResourceRecordSet(QObject * parent = nullptr, int capacity = 100);

    virtual ~ResourceRecordSet() override;

public:
    void commit(ResourceRecord * record);

    void prepare();

    void add(ResourceRecord * record);

    void commit(bool drop = false);

    void undo();

    void redo();

    bool inOperation() const;

private:
    QList<ResourceRecord*> records_;
    int prepare_ = 0;
    bool drop_ = false;
    ResourceRecord * record_ = nullptr;
    MergeRecord * mergeRecord_ = nullptr;
    int capacity_;
    int undo_ = 0;
    ResourceRecord * operation_ = nullptr;
};

class RecordMergeScope
{
public:
    RecordMergeScope(ResourceRecordSet * set)
        : set_(set)
    {
        if (set_)
            set_->prepare();
    }
    ~RecordMergeScope()
    {
        if (set_)
            set_->commit(drop_);
    }
    operator bool() const;
    void add(ResourceRecord * record);
    void drop() { drop_ = true; }
private:
    ResourceRecordSet * set_;
    bool drop_ = false;
};

#endif // RESOURCERECORD_H
