#include "resourcerecord.h"

ResourceRecordSet::ResourceRecordSet(QObject * parent, int capacity)
    : QObject(parent)
    , capacity_(capacity)
{
}

ResourceRecordSet::~ResourceRecordSet()
{
    clear();
}

void ResourceRecordSet::commit(ResourceRecord *record)
{
    if (undo_) {
        for (int i = records_.size() - undo_; i < records_.size(); ++i)
            delete records_[i];
        records_.erase(records_.begin() + records_.size() - undo_, records_.end());
        undo_ = 0;
    }
    records_.append(record);
    if (records_.size() > capacity_)
        delete records_.takeFirst();
}

void ResourceRecordSet::prepare()
{
    ++prepare_;
}

void ResourceRecordSet::add(ResourceRecord *record)
{
    if (prepare_ <= 0) {
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
    if (--prepare_ == 0 && record_) {
        if (drop_) {
            record_->undo();
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

void ResourceRecordSet::clear()
{
    for (int i = records_.size() - 1; i >= 0; --i)
        delete records_[i];
    records_.clear();
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

RecordMergeScope::RecordMergeScope(ResourceRecordSet *set)
    : set_(set)
{
    if (set_)
        set_->prepare();
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

RecordMergeScope::operator bool() const
{
    return set_ && !set_->inOperation();
}
