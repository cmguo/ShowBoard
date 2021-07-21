#include "control.h"
#include "resourcepackage.h"
#include "resourcepage.h"
#include "resourcerecord.h"
#include "resourceview.h"

#include <QDebug>

#ifdef QT_DEBUG
#define SHOWBOARD_RECORD_DUMP SHOWBOARD_RECORD
#else
#define SHOWBOARD_RECORD_DUMP 0
#endif

static bool sEnable = SHOWBOARD_RECORD;
static bool sEnableDump = SHOWBOARD_RECORD_DUMP;

static void initConfig()
{
    static bool inited = false;
    if (!inited) {
        QString record = qEnvironmentVariable("SHOWBOARD_RECORD");
        if (!record.isEmpty())
            sEnable = sEnableDump = QVariant(record).toBool();
        QString recordDump = qEnvironmentVariable("SHOWBOARD_RECORD_DUMP");
        if (!recordDump.isEmpty())
            sEnableDump = QVariant(record).toBool();
    }
}

/* ResourceRecordSet */

ResourceRecordSet::ResourceRecordSet(QObject * parent, int capacity)
    : QObject(parent)
    , capacity_(capacity)
{
    initConfig();
    if (!sEnable)
        capacity_ = 0;
}

ResourceRecordSet::~ResourceRecordSet()
{
    clear();
}

void ResourceRecordSet::commit(ResourceRecord *record)
{
    qDebug() << "ResourceRecordSet commit" << undo_ << records_.size() + 1 << record;
    record->dump();
    if (undo_) {
        clear(records_.size() - undo_);
        undo_ = 0;
    }
    records_.append(record);
    if (records_.size() > capacity_) {
        qDebug() << "ResourceRecordSet drop" << undo_ << records_.size() << record;
        delete records_.takeFirst();
    }
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

ResourceRecord *ResourceRecordSet::merge(ResourceRecord *dest, MergeRecord *&merge, ResourceRecord *record)
{
    if (merge)
        merge->add(record);
    else if (dest)
        dest = merge = new MergeRecord({dest, record});
    else
        dest = record;
    return dest;
}

void ResourceRecordSet::add(ResourceRecord *record)
{
    if (prepare_ <= 0 || blocked()) {
        delete record;
        return;
    }
    record_ = merge(record_, mergeRecord_, record);
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
        qDebug() << "ResourceRecordSet undo" << undo_ << records_.size() << operation_;
        operation_->dump();
        operation_->undo();
        operation_ = nullptr;
    }
}

void ResourceRecordSet::redo()
{
    if (undo_) {
        operation_ = records_[records_.size() - undo_];
        qDebug() << "ResourceRecordSet redo" << undo_ << records_.size() << operation_;
        operation_->dump();
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
    for (int i = keep; i < records_.size() - 1; ++i) {
        qDebug() << "ResourceRecordSet clear" << undo_ << records_.size() + 1 << records_[i];
        records_[i]->dump();
        delete records_[i];
    }
    records_.erase(records_.begin() + keep, records_.end());
}

/* MergeRecord */

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

void ResourceRecord::dump()
{
    if (sEnableDump)
        qDebug() << info_;
}

void MergeRecord::dump()
{
    if (sEnableDump)
        for (int i = 0; i < records_.size(); ++i)
            records_[i]->dump();
}

/* RecordMergeScope */

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
    : RecordMergeScope(page ? page->records() : nullptr, block)
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
    if (bool(*this))
        set_->add(record);
    else
        delete record;
}

void RecordMergeScope::drop()
{
    drop_ = true;
}

bool RecordMergeScope::atTop() const
{
    return set_ && set_->prepareLevel() == 1;
}

bool RecordMergeScope::inOperation() const
{
    return set_ && set_->inOperation();
}

RecordMergeScope::operator bool() const
{
    return sEnable && set_ && !set_->inOperation() && !set_->blocked();
}
