/**
 * index_iterator.cpp
 */
#include <cassert>

#include "index/index_iterator.h"

namespace scudb {

/*
 * NOTE: you can change the destructor/constructor method here
 * set your own input parameters
 */
    /**
     * @brief 构造函数
     * @tparam KeyType
     * @tparam ValueType
     * @tparam KeyComparator
     * @param leaf
     * @param index
     * @param buf_pool_manager
     */
    INDEX_TEMPLATE_ARGUMENTS
    INDEXITERATOR_TYPE::IndexIterator(B_PLUS_TREE_LEAF_PAGE_TYPE *leaf, int index, BufferPoolManager *buf_pool_manager)
            : mIndex_(index),mLeafPage_(leaf), mBufferPoolManager_(buf_pool_manager){}

            ////析构函数
    INDEX_TEMPLATE_ARGUMENTS
    INDEXITERATOR_TYPE::~IndexIterator() {
        if (mLeafPage_ != nullptr) {
            UnlockAndUnPin();
        }
    }

    /**
     * @brief : 解锁并且unpin，释放自己的锁
     */
    INDEX_TEMPLATE_ARGUMENTS
    void INDEXITERATOR_TYPE::UnlockAndUnPin() {
        mBufferPoolManager_->FetchPage(mLeafPage_->GetPageId())->RUnlatch();
        mBufferPoolManager_->UnpinPage(mLeafPage_->GetPageId(), false);
        mBufferPoolManager_->UnpinPage(mLeafPage_->GetPageId(), false);
    }

    INDEX_TEMPLATE_ARGUMENTS
    bool INDEXITERATOR_TYPE::isEnd(){
        if (mLeafPage_ == nullptr){
            return true;
        } else{
            return false;
        }
    }

    ////地址解析符*重载
    INDEX_TEMPLATE_ARGUMENTS
    const MappingType & INDEXITERATOR_TYPE::operator*() {
        return mLeafPage_->GetItem(mIndex_);
    }


    INDEX_TEMPLATE_ARGUMENTS
    INDEXITERATOR_TYPE& INDEXITERATOR_TYPE::operator++() {
        mIndex_++;
        if (mIndex_ >= mLeafPage_->GetSize()) {
            page_id_t next = mLeafPage_->GetNextPageId();
            UnlockAndUnPin();
            if (next == INVALID_PAGE_ID) {
                mLeafPage_ = nullptr;
            } else {
                Page *page = mBufferPoolManager_->FetchPage(next);
                page->RLatch();
                mLeafPage_ = reinterpret_cast<B_PLUS_TREE_LEAF_PAGE_TYPE *>(page->GetData());
                mIndex_ = 0;
            }
        }
        return *this;
    }

    template class IndexIterator<GenericKey<4>, RID, GenericComparator<4>>;
    template class IndexIterator<GenericKey<8>, RID, GenericComparator<8>>;
    template class IndexIterator<GenericKey<16>, RID, GenericComparator<16>>;
    template class IndexIterator<GenericKey<32>, RID, GenericComparator<32>>;
    template class IndexIterator<GenericKey<64>, RID, GenericComparator<64>>;

} // namespace scudb
