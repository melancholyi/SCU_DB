/**
 * index_iterator.h
 * For range scan of b+ tree
 */
#pragma once
#include "page/b_plus_tree_leaf_page.h"

namespace scudb {

#define INDEXITERATOR_TYPE                                                     \
  IndexIterator<KeyType, ValueType, KeyComparator>

    INDEX_TEMPLATE_ARGUMENTS
    class IndexIterator {
    public:
        // you may define your own constructor based on your member variables
        IndexIterator(B_PLUS_TREE_LEAF_PAGE_TYPE *leaf, int index, BufferPoolManager *buf_pool_manager);
        ~IndexIterator();


        //// public function
        bool isEnd();
        const MappingType &operator*();
        IndexIterator &operator++();




    private:////function
        // add your own private member variables here
        void UnlockAndUnPin();

    private:////membership

        int mIndex_;
        B_PLUS_TREE_LEAF_PAGE_TYPE *mLeafPage_;
        BufferPoolManager *mBufferPoolManager_;
    };

} // namespace scudb
