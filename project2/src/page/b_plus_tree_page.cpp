/**
 * b_plus_tree_page.cpp
 */
#include "page/b_plus_tree_page.h"

namespace scudb {

/*
 * Helper methods to get/set page type
 * Page type enum class is defined in b_plus_tree_page.h
 */
////判断是否为叶子page
bool BPlusTreePage::IsLeafPage() const {
    if(page_type_ == IndexPageType::LEAF_PAGE){
        return true;
    }else{
        return false;
    }
}

/**
 * @brief : 判断是否为root page
 * @return： bool
 */
bool BPlusTreePage::IsRootPage() const {
    if(parent_page_id_ == INVALID_PAGE_ID){
        return true;
    }
    else{
        return false;
    }
}

////设置pagetype parent or leaf or inter...
void BPlusTreePage::SetPageType(IndexPageType page_type) {
    page_type_ = page_type;
}

/*
 * Helper methods to get/set size (number of key/value pairs stored in that
 * page)
 */
int BPlusTreePage::GetSize() const {
    return  size_;
}

void BPlusTreePage::SetSize(int size) {
    size_ = size;
}
void BPlusTreePage::IncreaseSize(int amount) {
    size_ = size_ + amount;
}

/*
 * Helper methods to get/set max size (capacity) of the page
 */
int BPlusTreePage::GetMaxSize() const {
    return max_size_;
}
void BPlusTreePage::SetMaxSize(int size) {
    max_size_ = size;
}

/*
 * Helper method to get min page size
 * Generally, min page size == max page size / 2
 */
int BPlusTreePage::GetMinSize() const {
    if (IsRootPage()) {
        return IsLeafPage() ? 1: 2;
    }
    return (max_size_ ) / 2;
}

/*
 * Helper methods to get/set parent page id
 */
page_id_t BPlusTreePage::GetParentPageId() const {
    return parent_page_id_;
}

void BPlusTreePage::SetParentPageId(page_id_t parent_page_id) {
    parent_page_id_ = parent_page_id;
}

/*
 * Helper methods to get/set self page id
 */
page_id_t BPlusTreePage::GetPageId() const {
    return page_id_;
}

void BPlusTreePage::SetPageId(page_id_t page_id) {
    page_id_ = page_id;
}

/*
 * Helper methods to set lsn
 */
void BPlusTreePage::SetLSN(lsn_t lsn) {
    lsn_ = lsn;
}

bool BPlusTreePage::isSafe(eOpType op){
    int size = GetSize();
    ////插入模式
    if (op == eOpType::INSERT) {
        ////未越界
        if(size < this->GetMaxSize()){
            return true;
        }
        else {
            return false;
        }
    }
    int min_size = GetMinSize() + 1;
    ////B+树删除模式
    if (op == eOpType::DELETE) {
        return (IsLeafPage()) ? size >= min_size : size > min_size;
    }
    ////否则是为不安全，进行assert，出错直接让其崩溃
    assert(false);
}

} // namespace scudb
