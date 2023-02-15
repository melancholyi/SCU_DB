/**
 * b_plus_tree.h
 *
 * Implementation of simple b+ tree data structure where internal pages direct
 * the search and leaf pages contain actual data.
 * (1) We only support unique key
 * (2) support insert & remove
 * (3) The structure should shrink and grow dynamically
 * (4) Implement index iterator for range scan
 */
#pragma once

#include <queue>
#include <vector>

#include "concurrency/transaction.h"
#include "index/index_iterator.h"
#include "page/b_plus_tree_internal_page.h"
#include "page/b_plus_tree_leaf_page.h"

namespace scudb {

#define BPLUSTREE_TYPE BPlusTree<KeyType, ValueType, KeyComparator>
// Main class providing the API for the Interactive B+ Tree.
    INDEX_TEMPLATE_ARGUMENTS
    class BPlusTree {
    public:
        ////构造函数
        explicit BPlusTree(const std::string &name,
                           BufferPoolManager *buffer_pool_manager,
                           const KeyComparator &comparator,
                           page_id_t root_page_id = INVALID_PAGE_ID);

        // Returns true if this B+ tree has no keys and values.
        bool IsEmpty() const;////

        // Insert a key-value pair into this B+ tree.
        bool Insert(const KeyType &key, const ValueType &value,
                    Transaction *transaction = nullptr);

        // Remove a key and its value from this B+ tree.
        void Remove(const KeyType &key, Transaction *transaction = nullptr);

        // return the value associated with a given key
        bool GetValue(const KeyType &key, std::vector<ValueType> &result,
                      Transaction *transaction = nullptr);

        // index iterator
        INDEXITERATOR_TYPE Begin();
        INDEXITERATOR_TYPE Begin(const KeyType &key);

        // Print this B+ tree to stdout using a simple command-line
        std::string ToString(bool verbose = false);

        // read data from file and insert one by one
        void InsertFromFile(const std::string &file_name,
                            Transaction *transaction = nullptr);

        // read data from file and remove one by one
        void RemoveFromFile(const std::string &file_name,
                            Transaction *transaction = nullptr);
        // expose for test purpose
        B_PLUS_TREE_LEAF_PAGE_TYPE *FindLeafPage(const KeyType &key,
                                                 bool leftMost = false,
                                                 eOpType op = eOpType::READ,
                                                 Transaction *transaction = nullptr);
        // expose for test purpose
        bool Check(bool force = false);
        bool openCheck = true;
    private:
        BPlusTreePage *FetchPage(page_id_t page_id);

        void StartNewTree(const KeyType &key, const ValueType &value);

        bool InsertIntoLeaf(const KeyType &key, const ValueType &value,
                            Transaction *transaction = nullptr);

        void InsertIntoParent(BPlusTreePage *old_node, const KeyType &key,
                              BPlusTreePage *new_node,
                              Transaction *transaction = nullptr);

        template <typename N> N *Split(N *node, Transaction *transaction);

        template <typename N>
        bool CoalesceOrRedistribute(N *node, Transaction *transaction = nullptr);

        template <typename N>
        bool FindLeftSibling(N *node, N * &sibling, Transaction *transaction);

        template <typename N>
        bool Coalesce(
                N *&neighbor_node, N *&node,
                BPlusTreeInternalPage<KeyType, page_id_t, KeyComparator> *&parent,
                int index, Transaction *transaction = nullptr);

        template <typename N> void Redistribute(N *neighbor_node, N *node, int index);

        bool AdjustRoot(BPlusTreePage *node);

        void UpdateRootPageId(int insert_record = false);

        ////my helper function begin
        void Lock(bool exclusive,Page * page) ;
        void Unlock(bool exclusive,Page * page) ;
        void Unlock(bool exclusive,page_id_t pageId);
        void LockRootPageId(bool exclusive) ;
        void TryUnlockRootPageId(bool exclusive) ;

        BPlusTreePage *CrabingProtocalFetchPage(page_id_t page_id,eOpType op, page_id_t previous, Transaction *transaction);
        void FreePagesInTransaction(bool exclusive,  Transaction *transaction, page_id_t cur = -1);

        int isBalanced(page_id_t pid);
        bool isPageCorr(page_id_t pid,std::pair<KeyType,KeyType> &out);

    private:////membership variable
        // member variable
        std::string index_name_;
        page_id_t root_page_id_;
        BufferPoolManager *buffer_pool_manager_;
        KeyComparator comparator_;

        ////my private membership
        RWMutex mMutex_;
        static thread_local int mRootLockedCnt;

    };
} // namespace scudb