#include "control.h"
#include "resourcepackage.h"
#include "resourcepage.h"
#include "resourcerecord.h"
#include "resourceview.h"

ResourceRecordSet::ResourceRecordSet(QObject * parent, int capacity)
    : QObject(parent)
    , capacity_(capacity)
{
#if !SHOWBOARD_RECORD
    capacity_ = 0;
#endif
}

ResourceRecordSet::~ResourceRecordSet()
{
    clear();
}

void ResourceRecordSet::commit(ResourceRecord *record)
{
    if (undo_) {
        clear(records_.size() - undo_);
        undo_ = 0;
    }
    records_.append(record);
    if (records_.size() > capacity_)
        delete records_.takeFirst();
}

void ResourceRecordSet::prepare(bool block)
{
    if (prepare_ == 0 && !operation_ && undo_) {
        clear(records_.size() - undo_);
        undo_ = 0;
    }
    ++prepare_;
    if (block && block_ == -1)
        block_ = prepare_;
}

void ResourceRecordSet::add(ResourceRecord *record)
{
    if (prepare_ <= 0 || blocked()) {
        delete record;
        return;
    }
    if (mergeRecord_)
        mergeRecord_->add(record);
    else if (record_)
        record_ = mergeRecord_ = new MergeRecord({record_, record});
    else
        record_ = record;
}

void ResourceRecordSet::commit(bool drop)
{
    drop_ |= drop;
    if (prepare_ == block_)
        block_ = -1;
    if (--prepare_ == 0 && record_) {
        if (drop_) {
            delete record_;
        } else {
            commit(record_);
        }
        record_ = mergeRecord_ = nullptr;
        drop_ = false;
    }
}

void ResourceRecordSet::undo()
{
    if (undo_ < records_.size()) {
        ++undo_;
        operation_ = records_[records_.size() - undo_];
        operation_->undo();
        operation_ = nullptr;
    }
}

void ResourceRecordSet::redo()
{
    if (undo_) {
        operation_ = records_[records_.size() - undo_];
        operation_->redo();
        operation_ = nullptr;
        --undo_;
    }
}

bool ResourceRecordSet::inOperation() const
{
    return operation_ != nullptr;
}

void ResourceRecordSet::clear(int keep)
{
    for (int i = records_.size() - 1; i >= keep; --i)
        delete records_[i];
    records_.erase(records_.begin() + keep, records_.end());
}

MergeRecord::MergeRecord(QList<ResourceRecord *> const & records)
    : records_(records)
{
}

MergeRecord::~MergeRecord()
{
    while (!records_.empty()) {
        delete records_.takeLast();
    }
}

void MergeRecord::add(ResourceRecord *record)
{
    records_.append(record);
}

void MergeRecord::undo()
{
    for (int i = records_.size() - 1; i >= 0; --i)
        records_[i]->undo();
}

void MergeRecord::redo()
{
    for (int i = 0; i < records_.size(); ++i)
        records_[i]->redo();
}

RecordMergeScope::RecordMergeScope(ResourceRecordSet *set, bool block)
    : set_(set)
{
    if (set_)
        set_->prepare(block);
}

RecordMergeScope::RecordMergeScope(ResourcePackage * pkg, bool block)
: RecordMergeScope(pkg ? pkg->records() : nullptr, block)
{
}

RecordMergeScope::RecordMergeScope(ResourcePage *page, bool block)
    : RecordMergeScope(page ? page->package() : nullptr, block)
{
}

RecordMergeScope::RecordMergeScope(ResourceView *res, bool block)
    : RecordMergeScope(res ? res->page() : nullptr, block)
{
}

RecordMergeScope::RecordMergeScope(Control *ctrl, bool block)
    : RecordMergeScope(ctrl ? ctrl->resource() : nullptr, block)
{
}

RecordMergeScope::RecordMergeScope(ControlView *view, bool block)
    : RecordMergeScope(Control::fromChildItem(view), block)
{
}

RecordMergeScope::~RecordMergeScope()
{
    if (set_)
        set_->commit(drop_);
}

void RecordMergeScope::add(ResourceRecord *record)
{
    if (set_)
        set_->add(record);
}

void RecordMergeScope::drop()
{
    drop_ = true;
}

bool RecordMergeScope::atTop() const
{
    return set_ && set_->prepareLevel() == 1;
}

RecordMergeScope::operator bool() const
{
    return set_ && !set_->inOperation() && !set_->blocked();
}
