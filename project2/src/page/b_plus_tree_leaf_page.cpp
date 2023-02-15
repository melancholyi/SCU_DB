/**
 * b_plus_tree_leaf_page.cpp
 */

#include <sstream>

#include "common/exception.h"
#include "common/rid.h"
#include "page/b_plus_tree_leaf_page.h"
#include "page/b_plus_tree_internal_page.h"

namespace scudb {

/*****************************************************************************
 * HELPER METHODS AND UTILITIES
 *****************************************************************************/

/**
 * Init method after creating a new leaf page
 * Including set page type, set current size to zero, set page id/parent id, set
 * next page id and set max size
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_LEAF_PAGE_TYPE::Init(page_id_t page_id, page_id_t parent_id) {
    SetPageType(IndexPageType::LEAF_PAGE);////叶子节点

    ////init membership value
    SetSize(0);
    assert(sizeof(BPlusTreeLeafPage) == 28);
    SetPageId(page_id);
    SetParentPageId(parent_id);
    SetNextPageId(INVALID_PAGE_ID);
    int max_size = (PAGE_SIZE - sizeof(BPlusTreeLeafPage))/sizeof(MappingType);
    SetMaxSize(max_size - 1);
}

/**
 * Helper methods to set/get next page id
 */
INDEX_TEMPLATE_ARGUMENTS
page_id_t B_PLUS_TREE_LEAF_PAGE_TYPE::GetNextPageId() const {
    return next_page_id_;
}

INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_LEAF_PAGE_TYPE::SetNextPageId(page_id_t next_page_id) {
    next_page_id_ = next_page_id;
}

/**
 * Helper method to find the first index i so that array[i].first >= key
 * NOTE: This method is only used when generating index iterator
 */
INDEX_TEMPLATE_ARGUMENTS
int B_PLUS_TREE_LEAF_PAGE_TYPE::KeyIndex(
    const KeyType &key, const KeyComparator &comparator) const {

    int begin= 0, end = GetSize() - 1;
    ////二分查找 因为B+树是一个平衡树！！
    while (begin <= end) {
        int mid = (end - begin) / 2 + begin;
        ////前一半寻找
        if (comparator(array[mid].first,key) >= 0){
            end = mid - 1;
        }
        ////后一半寻找
        else begin= mid + 1;
    }
    return end + 1;
}

/*
 * Helper method to find and return the key associated with input "index"(a.k.a
 * array offset)
 */
INDEX_TEMPLATE_ARGUMENTS
KeyType B_PLUS_TREE_LEAF_PAGE_TYPE::KeyAt(int index) const {
    assert(index >= 0 && index < GetSize());
    return array[index].first;
}

/*
 * Helper method to find and return the key & value pair associated with input
 * "index"(a.k.a array offset)
 */
INDEX_TEMPLATE_ARGUMENTS
const MappingType &B_PLUS_TREE_LEAF_PAGE_TYPE::GetItem(int index) {
    // replace with your own code
    return array[index];////返回数据，pair类型的重载[]
}

/*****************************************************************************
 * INSERTION
 *****************************************************************************/
/*
 * Insert key & value pair into leaf page ordered by key
 * @return  page size after insertion
 */
INDEX_TEMPLATE_ARGUMENTS
int B_PLUS_TREE_LEAF_PAGE_TYPE::Insert(const KeyType &key,
                                       const ValueType &value,
                                       const KeyComparator &comparator) {
    int index = KeyIndex(key,comparator); //first larger than key
    assert(index >= 0);
    IncreaseSize(1);
    int cur_size = GetSize();
    ////向后移动腾出位置
    for (int i = cur_size - 1; i > index; i--) {
        array[i].first = array[i - 1].first;
        array[i].second = array[i - 1].second;
    }
    ////插入新键值对
    array[index].first = key;
    array[index].second = value;
    return cur_size;
}

/*****************************************************************************
 * SPLIT
 *****************************************************************************/
/*
 * Remove half of key & value pairs from this page to "recipient" page
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_LEAF_PAGE_TYPE::MoveHalfTo(
        BPlusTreeLeafPage *recipient,
        __attribute__((unused)) BufferPoolManager *buffer_pool_manager) {
    int total = GetMaxSize() + 1;

    ////复制后一半，ceil(total) + 1
    int mid_index = (total)/2;

    ////复制array的后一般到recipient中
    for (int i = mid_index; i < total; i++) {
        recipient->array[i - mid_index].first = array[i].first;
        recipient->array[i - mid_index].second = array[i].second;
    }
    ////设置相关的指针变化
    recipient->SetNextPageId(GetNextPageId());
    SetNextPageId(recipient->GetPageId());

    SetSize(mid_index);
    recipient->SetSize(total - mid_index);

}

INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_LEAF_PAGE_TYPE::CopyHalfFrom(MappingType *items, int size) {}

/*****************************************************************************
 * LOOKUP
 *****************************************************************************/
/*
 * For the given key, check to see whether it exists in the leaf page. If it
 * does, then store its corresponding value in input "value" and return true.
 * If the key does not exist, then return false
 */
INDEX_TEMPLATE_ARGUMENTS
bool B_PLUS_TREE_LEAF_PAGE_TYPE::Lookup(const KeyType &key, ValueType &value,
                                        const KeyComparator &comparator) const {
    int index = KeyIndex(key,comparator);
    if (index < GetSize() && comparator(array[index].first, key) == 0) {
        value = array[index].second;
        return true;
    }
    return false;
}

/*****************************************************************************
 * REMOVE
 *****************************************************************************/
/*
 * First look through leaf page to see whether delete key exist or not. If
 * exist, perform deletion, otherwise return immdiately.
 * NOTE: store key&value pair continuously after deletion
 * @return   page size after deletion
 */
INDEX_TEMPLATE_ARGUMENTS
int B_PLUS_TREE_LEAF_PAGE_TYPE::RemoveAndDeleteRecord(
        const KeyType &key, const KeyComparator &comparator) {

    int tar_key = KeyIndex(key,comparator);
    ////不存在这个tar_key
    if (tar_key >= GetSize() || comparator(key,KeyAt(tar_key)) != 0) {
        return GetSize();
    }
    else{
        //quick deletion
        int tar_index = tar_key;
        ////直接使用内存复制函数，将后续数据直接向前移动覆盖掉tar_index的数据
        memmove(array + tar_index, array + tar_index + 1,
                static_cast<size_t>((GetSize() - tar_index - 1)*sizeof(MappingType)));
        IncreaseSize(-1); // remove size--
        return GetSize();
    }

}

/*****************************************************************************
 * MERGE
 *****************************************************************************/
/*
 * Remove all of key & value pairs from this page to "recipient" page, then
 * update next page id
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_LEAF_PAGE_TYPE::MoveAllTo(BPlusTreeLeafPage *recipient,
                                           int, BufferPoolManager *) {
    ////复制所有的数据到recipient
    int start_index = recipient->GetSize();
    for (int i = 0; i < GetSize(); i++) {
        recipient->array[start_index + i].first = array[i].first;
        recipient->array[start_index + i].second = array[i].second;
    }
    ////链接指针
    recipient->SetNextPageId(GetNextPageId());
    ////更新size大小
    recipient->IncreaseSize(GetSize());
    SetSize(0);

}
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_LEAF_PAGE_TYPE::CopyAllFrom(MappingType *items, int size) {}

/*****************************************************************************
 * REDISTRIBUTE
 *****************************************************************************/
/*
 * Remove the first key & value pair from this page to "recipient" page, then
 * update relavent key & value pair in its parent page.
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_LEAF_PAGE_TYPE::MoveFirstToEndOf(
        BPlusTreeLeafPage *recipient,
        BufferPoolManager *buffer_pool_manager) {
    MappingType pair = GetItem(0);
    IncreaseSize(-1);
    ////内存拷贝，快速移动数据
    memmove(array, array + 1, static_cast<size_t>(GetSize()*sizeof(MappingType)));
    recipient->CopyLastFrom(pair);

    ////更新parent page中相关的键值对
    Page *page = buffer_pool_manager->FetchPage(GetParentPageId());
    auto *parent = reinterpret_cast<B_PLUS_TREE_INTERNAL_PAGE *>(page->GetData());
    parent->SetKeyAt(parent->ValueIndex(GetPageId()), array[0].first);
    buffer_pool_manager->UnpinPage(GetParentPageId(), true);
}

INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_LEAF_PAGE_TYPE::CopyLastFrom(const MappingType &item) {
    array[GetSize()] = item;
    IncreaseSize(1);
}
/*
 * Remove the last key & value pair from this page to "recipient" page, then
 * update relavent key & value pair in its parent page.
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_LEAF_PAGE_TYPE::MoveLastToFrontOf(
        BPlusTreeLeafPage *recipient, int parentIndex,
        BufferPoolManager *buffer_pool_manager) {
    MappingType pair = GetItem(GetSize() - 1);////得到最后一个pair键值对
    IncreaseSize(-1);
    ////pair拼接到recopient的头
    recipient->CopyFirstFrom(pair, parentIndex, buffer_pool_manager);
}

INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_LEAF_PAGE_TYPE::CopyFirstFrom(
        const MappingType &item, int parentIndex,
        BufferPoolManager *buffer_pool_manager) {

    ////内存移动，快速删除
    memmove(array + 1, array, GetSize()*sizeof(MappingType));
    IncreaseSize(1);////++
    array[0] = item;

    ////更新parent相关的键值对
    Page *page = buffer_pool_manager->FetchPage(GetParentPageId());
    auto *parent = reinterpret_cast<B_PLUS_TREE_INTERNAL_PAGE *>(page->GetData());
    parent->SetKeyAt(parentIndex, array[0].first);
    buffer_pool_manager->UnpinPage(GetParentPageId(), true);
}

/*****************************************************************************
 * DEBUG
 *****************************************************************************/
INDEX_TEMPLATE_ARGUMENTS
std::string B_PLUS_TREE_LEAF_PAGE_TYPE::ToString(bool verbose) const {
    if (GetSize() == 0) {
        return "";
    }
    std::ostringstream stream;
    if (verbose) {
        stream << "[pageId: " << GetPageId() << " parentId: " << GetParentPageId()
               << "]<" << GetSize() << "> ";
    }
    int entry = 0;
    int end = GetSize();
    bool first = true;

    while (entry < end) {
        if (first) {
            first = false;
        } else {
            stream << " ";
        }
        stream << std::dec << array[entry].first;
        if (verbose) {
            stream << "(" << array[entry].second << ")";
        }
        ++entry;
    }
    return stream.str();
}

template class BPlusTreeLeafPage<GenericKey<4>, RID,
        GenericComparator<4>>;
template class BPlusTreeLeafPage<GenericKey<8>, RID,
        GenericComparator<8>>;
template class BPlusTreeLeafPage<GenericKey<16>, RID,
        GenericComparator<16>>;
template class BPlusTreeLeafPage<GenericKey<32>, RID,
        GenericComparator<32>>;
template class BPlusTreeLeafPage<GenericKey<64>, RID,
        GenericComparator<64>>;
} // namespace scudb
