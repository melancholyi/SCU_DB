/**
 * b_plus_tree.cpp
 */
#include <iostream>
#include <string>

#include "common/exception.h"
#include "common/logger.h"
#include "common/rid.h"
#include "index/b_plus_tree.h"
#include "page/header_page.h"

namespace scudb {
    using namespace std;
    ////构造函数
    INDEX_TEMPLATE_ARGUMENTS
    BPLUSTREE_TYPE::BPlusTree(const std::string &name, ////B+tree‘s name
                              BufferPoolManager *buffer_pool_manager, ////缓冲池
                              const KeyComparator &comparator,
                              page_id_t root_page_id) ////tree rootpage号
            : index_name_(name), root_page_id_(root_page_id),
              buffer_pool_manager_(buffer_pool_manager), comparator_(comparator) {}

/*
 * Helper function to decide whether current b+tree is empty
 */
    INDEX_TEMPLATE_ARGUMENTS
    bool BPLUSTREE_TYPE::IsEmpty() const {
        if(root_page_id_ == INVALID_PAGE_ID){
            return true;
        }else{
            return false;
        }
    }


/*****************************************************************************
 * SEARCH
 *****************************************************************************/
/*
 * Return the only value that associated with input key
 * This method is used for point query
 * @return : true means key exists
 */
    INDEX_TEMPLATE_ARGUMENTS
    bool BPLUSTREE_TYPE::GetValue(const KeyType &key,
                                  std::vector<ValueType> &result,
                                  Transaction *transaction) {

        ////B+树中value都存储在叶子节点，故先找到它
        B_PLUS_TREE_LEAF_PAGE_TYPE *tar_page = FindLeafPage(key,false,eOpType::READ,transaction);
        if (tar_page == nullptr)
            return false;
        else {////正确找到
            //find value
            result.resize(1);
            ////调用leafpage的函数lokup寻找value
            auto ret_val = tar_page->Lookup(key,result[0],comparator_);

            ////操作完毕，unpinPage
            FreePagesInTransaction(false,transaction,tar_page->GetPageId());
            return ret_val;
        }

    }

/*****************************************************************************
 * INSERTION
 *****************************************************************************/
/*
 * Insert constant key & value pair into b+ tree
 * if current tree is empty, start new tree, update root page id and insert
 * entry, otherwise insert into leaf page.
 * @return: since we only support unique key, if user try to insert duplicate
 * keys return false, otherwise return true.
 */
INDEX_TEMPLATE_ARGUMENTS
bool BPLUSTREE_TYPE::Insert(const KeyType &key, const ValueType &value,
                            Transaction *transaction) {
    ////insert从根部寻找插入index
    LockRootPageId(true);////先找到根的page_id
    if (IsEmpty()) {////树为空，则创建新树
        StartNewTree(key,value);
        TryUnlockRootPageId(true);
        return true;
    }else{////树不为空
        TryUnlockRootPageId(true);
        ////调用insert函数进行插入
        bool is_success = InsertIntoLeaf(key,value,transaction);
        return is_success;
    }
}
/*
 * Insert constant key & value pair into an empty tree
 * User needs to first ask for new page from buffer pool manager(NOTICE: throw
 * an "out of memory" exception if returned value is nullptr), then update b+
 * tree's root page id and insert entry directly into leaf page.
 */
    ////从根创建新树
    INDEX_TEMPLATE_ARGUMENTS
    void BPLUSTREE_TYPE::StartNewTree(const KeyType &key, const ValueType &value) {

        //// 首先从buffer——pool处申请个new page
        page_id_t id;
        Page *root_page = buffer_pool_manager_->NewPage(id);
        //// 构建B+tree root
        auto *root = reinterpret_cast<B_PLUS_TREE_LEAF_PAGE_TYPE *>(root_page->GetData());

        ////更新B+数根部的page——id
        root->Init(id,INVALID_PAGE_ID);////调用init函数初始化B+树的leaf——page
        root_page_id_ = id;
        UpdateRootPageId(true);//调用函数，更新

        ////调用insert函数，插入要加到新tree中的键值对
        root->Insert(key,value,comparator_);

        ////unpin，接触封锁
        buffer_pool_manager_->UnpinPage(id,true);
    }

/*
 * Insert constant key & value pair into leaf page
 * User needs to first find the right leaf page as insertion target, then look
 * through leaf page to see whether insert key exist or not. If exist, return
 * immdiately, otherwise insert entry. Remember to deal with split if necessary.
 * @return: since we only support unique key, if user try to insert duplicate
 * keys return false, otherwise return true.
 */
    ////函数作用：向leaf中插入键值对
    INDEX_TEMPLATE_ARGUMENTS
    bool BPLUSTREE_TYPE::InsertIntoLeaf(const KeyType &key, const ValueType &value,
                                        Transaction *transaction) {
        ////首先找到一个leaf page，这里选择insert插入模式
        auto *leaf_page = FindLeafPage(key,false,eOpType::INSERT,transaction);
        ValueType v;
        ////查看是否存在此键值对
        bool exist = leaf_page->Lookup(key,v,comparator_);
        if (exist) {////存在此键值对，插入失败
            FreePagesInTransaction(true,transaction);
            return false;
        } else { ////不存在，可以插入
            ////调用函数插入此键值对
            leaf_page->Insert(key,value,comparator_);

            if (leaf_page->GetSize() > leaf_page->GetMaxSize()) {//insert then split
                ////插入后判断超过一个page的最大size，所以需要分裂
                auto *new_leaf_page = Split(leaf_page,transaction);//调用分裂函数

                ////将page序列中间的键值插入到parent处
                InsertIntoParent(leaf_page,new_leaf_page->KeyAt(0),new_leaf_page,transaction);
            }

            FreePagesInTransaction(true,transaction);
            return true;
        }

    }

/*
 * Split input page and return newly created page.
 * Using template N to represent either internal page or leaf page.
 * User needs to first ask for new page from buffer pool manager(NOTICE: throw
 * an "out of memory" exception if returned value is nullptr), then move half
 * of key & value pairs from input page to newly created page
 */
    INDEX_TEMPLATE_ARGUMENTS
    template <typename N> N *BPLUSTREE_TYPE::Split(N *node, Transaction *transaction) {
        ////page超则进行分裂，必须继续保证平衡树的要求
        ////将中间键值对插入到父节点
        ////申请新page，中间往后的需要放在新的page，连在parent的右侧

        page_id_t new_page_id;
        Page* const new_page = buffer_pool_manager_->NewPage(new_page_id);

        new_page->WLatch();////排他锁封锁
        transaction->AddIntoPageSet(new_page);

        ////分裂，
        ////初始化新的节点，并将原来的page中中间往后的放在新的page中
        N *new_node = reinterpret_cast<N *>(new_page->GetData());
        new_node->Init(new_page_id, node->GetParentPageId());
        node->MoveHalfTo(new_node, buffer_pool_manager_);

        ////返回新创建的node
        return new_node;
    }

/*
 * Insert key & value pair into internal page after split
 * @param   old_node      input page from split() method
 * @param   key
 * @param   new_node      returned page from split() method
 * User needs to first find the parent page of old_node, parent node must be
 * adjusted to take info of new_node into account. Remember to deal with split
 * recursively if necessary.
 */
    INDEX_TEMPLATE_ARGUMENTS
    void BPLUSTREE_TYPE::InsertIntoParent(BPlusTreePage *old_node,
                                          const KeyType &key,
                                          BPlusTreePage *new_node,
                                          Transaction *transaction) {
        ////插入键值对到parent，在insert和分裂时使用
        if (old_node->IsRootPage()) {////元节点是父节点
            Page* const new_page = buffer_pool_manager_->NewPage(root_page_id_);

            ////群殴那个球新page，然后初始化并构造root节点
            auto *new_root = reinterpret_cast<B_PLUS_TREE_INTERNAL_PAGE *>(new_page->GetData());
            new_root->Init(root_page_id_);
            new_root->PopulateNewRoot(old_node->GetPageId(),key,new_node->GetPageId());

            ////更新相关数据
            old_node->SetParentPageId(root_page_id_);
            new_node->SetParentPageId(root_page_id_);
            UpdateRootPageId();

            //buffer_pool unpin
            buffer_pool_manager_->UnpinPage(new_root->GetPageId(),true);
            return;
        }
        else{////parent不是父节点
            page_id_t par_id = old_node->GetParentPageId();
            ////向上插入
            auto *page = FetchPage(par_id);
            auto *parent = reinterpret_cast<B_PLUS_TREE_INTERNAL_PAGE *>(page);
            new_node->SetParentPageId(par_id);
            parent->InsertNodeAfter(old_node->GetPageId(), key, new_node->GetPageId());

            if (parent->GetSize() > parent->GetMaxSize()) {////插入后超出容量则spilt
                auto *new_leaf_page = Split(parent,transaction);//new page need unpin
                InsertIntoParent(parent,new_leaf_page->KeyAt(0),new_leaf_page,transaction);
            }
            buffer_pool_manager_->UnpinPage(par_id,true);
        }

    }

/*****************************************************************************
 * REMOVE
 *****************************************************************************/
/*
 * Delete key & value pair associated with input key
 * If current tree is empty, return immdiately.
 * If not, User needs to first find the right leaf page as deletion target, then
 * delete entry from leaf page. Remember to deal with redistribute or merge if
 * necessary.
 */
    INDEX_TEMPLATE_ARGUMENTS
    void BPLUSTREE_TYPE::Remove(const KeyType &key, Transaction *transaction) {
        if (IsEmpty()) return;///空则直接return
        else{
            ////以Delete模式寻找target page
            auto *tar = FindLeafPage(key,false,eOpType::DELETE,transaction);
            ////remove
            int cur_size = tar->RemoveAndDeleteRecord(key,comparator_);
            if (cur_size < tar->GetMinSize()) {////不满足B+树的最小键值对数量要求，则需要合并or再分配
                CoalesceOrRedistribute(tar,transaction);
            }
            FreePagesInTransaction(true,transaction);
        }

    }

/*
 * User needs to first find the sibling of input page. If sibling's size + input
 * page's size > page's max size, then redistribute. Otherwise, merge.
 * Using template N to represent either internal page or leaf page.
 * @return: true means target leaf page should be deleted, false means no
 * deletion happens
 */
    INDEX_TEMPLATE_ARGUMENTS
    template <typename N>
    bool BPLUSTREE_TYPE::CoalesceOrRedistribute(N *node, Transaction *transaction) {

        if (node->IsRootPage()) { ////node节点为root根，则删除旧根
            bool delOldRoot = AdjustRoot(node);////调用函数，删除旧根，让旧根的孩子成为新root
            if (delOldRoot) {transaction->AddIntoDeletedPageSet(node->GetPageId());}
            return delOldRoot;
        }

        N *node2;
        bool is_r_sibling = FindLeftSibling(node,node2,transaction); ////查找node的左兄弟，赋值给node2
        BPlusTreePage *parent = FetchPage(node->GetParentPageId());
        auto *parent_page = static_cast<B_PLUS_TREE_INTERNAL_PAGE *>(parent);

        ////N和N2中的条目可以放在单个节点中
        if (node->GetSize() + node2->GetSize() <= node->GetMaxSize()) {
            if (is_r_sibling) {swap(node,node2);} ////假设node在node2之后
            int remove_index = parent_page->ValueIndex(node->GetPageId());
            ////调用合并函数，合并node和node2(分裂后保证B+树的要求的调整策略)
            Coalesce(node2,node,parent_page,remove_index,transaction);
            buffer_pool_manager_->UnpinPage(parent_page->GetPageId(), true);
            return true;////return ture means：合并
        }else {
            ////重新分配，分裂时遇见的另一种情况，孩子无法合并也不满足size要求，只能从父节点借一个
            int index_in_parent = parent_page->ValueIndex(node->GetPageId());
            Redistribute(node2,node,index_in_parent);////调用重新分配函数
            buffer_pool_manager_->UnpinPage(parent_page->GetPageId(), false);
            return false;////return false means：重新分配
        }

    }

    INDEX_TEMPLATE_ARGUMENTS
    template <typename N>
    bool BPLUSTREE_TYPE::FindLeftSibling(N *node, N * &sibling, Transaction *transaction) {
        auto page = FetchPage(node->GetParentPageId()); ////首先得到node的parent page
        auto *parent = reinterpret_cast<B_PLUS_TREE_INTERNAL_PAGE *>(page);
        int index = parent->ValueIndex(node->GetPageId());////parent index

        int siblingIndex = index - 1;
        ////无左兄弟，则找右兄弟
        if (index == 0) {
            siblingIndex = index + 1;
        }
        ////调用函数寻找兄弟
        sibling = reinterpret_cast<N *>(CrabingProtocalFetchPage(
                parent->ValueAt(siblingIndex),eOpType::DELETE,-1,transaction));
        buffer_pool_manager_->UnpinPage(parent->GetPageId(), false);
        if(index == 0){////返回true，表示是右兄弟
            return true;
        }else {
            return false;
        }
    }

/*
 * Move all the key & value pairs from one page to its sibling page, and notify
 * buffer pool manager to delete this page. Parent page must be adjusted to
 * take info of deletion into account. Remember to deal with coalesce or
 * redistribute recursively if necessary.
 * Using template N to represent either internal page or leaf page.
 * @param   neighbor_node      sibling page of input "node"
 * @param   node               input from method coalesceOrRedistribute()
 * @param   parent             parent page of input "node"
 * @return  true means parent node should be deleted, false means no deletion
 * happend
 */
    INDEX_TEMPLATE_ARGUMENTS
    template <typename N>
    bool BPLUSTREE_TYPE::Coalesce(
            N *&neighbor_node, N *&node,
            BPlusTreeInternalPage<KeyType, page_id_t, KeyComparator> *&parent,
            int index, Transaction *transaction) {

        ////合并兄弟page，移动node的全部键值对到兄弟node
        node->MoveAllTo(neighbor_node,index,buffer_pool_manager_);
        transaction->AddIntoDeletedPageSet(node->GetPageId());
        parent->Remove(index);
        if (parent->GetSize() <= parent->GetMinSize()) {
            return CoalesceOrRedistribute(parent,transaction);
        } else {
            return false;
        }
    }

/*
 * Redistribute key & value pairs from one page to its sibling page. If index ==
 * 0, move sibling page's first key & value pair into end of input "node",
 * otherwise move sibling page's last key & value pair into head of input
 * "node".
 * Using template N to represent either internal page or leaf page.
 * @param   neighbor_node      sibling page of input "node"
 * @param   node               input from method coalesceOrRedistribute()
 */
    INDEX_TEMPLATE_ARGUMENTS
    template <typename N>
    void BPLUSTREE_TYPE::Redistribute(N *neighbor_node, N *node, int index) {
        ////重新分配
        if (index == 0) neighbor_node->MoveFirstToEndOf(node,buffer_pool_manager_);
        else    neighbor_node->MoveLastToFrontOf(node, index, buffer_pool_manager_);
    }
/*
 * Update root page if necessary
 * NOTE: size of root page can be less than min size and this method is only
 * called within coalesceOrRedistribute() method
 * case 1: when you delete the last element in root page, but root page still
 * has one last child
 * case 2: when you delete the last element in whole b+ tree
 * @return : true means root page should be deleted, false means no deletion
 * happend
 */
    INDEX_TEMPLATE_ARGUMENTS
    bool BPLUSTREE_TYPE::AdjustRoot(BPlusTreePage *old_root_node) {
        ////更新root节点
        if (old_root_node->IsLeafPage()) {// case 2
            root_page_id_ = INVALID_PAGE_ID;
            UpdateRootPageId();
            return true;
        }
        if (old_root_node->GetSize() == 1) {// case 1
            auto *root = reinterpret_cast<B_PLUS_TREE_INTERNAL_PAGE *>(old_root_node);
            const page_id_t newRootId = root->RemoveAndReturnOnlyChild();
            root_page_id_ = newRootId;
            UpdateRootPageId();
            Page *page = buffer_pool_manager_->FetchPage(newRootId);
            assert(page != nullptr);
            auto *new_root =
                    reinterpret_cast<B_PLUS_TREE_INTERNAL_PAGE *>(page->GetData());
            new_root->SetParentPageId(INVALID_PAGE_ID);
            buffer_pool_manager_->UnpinPage(newRootId, true);
            return true;
        }
        return false;
    }

/*****************************************************************************
 * INDEX ITERATOR
 *****************************************************************************/
/*
 * Input parameter is void, find the leaftmost leaf page first, then construct
 * index iterator
 * @return : index iterator
 */
    INDEX_TEMPLATE_ARGUMENTS
    INDEXITERATOR_TYPE BPLUSTREE_TYPE::Begin() {
        KeyType unuse{};
        auto start_leaf = FindLeafPage(unuse, true);
        TryUnlockRootPageId(false);
        return INDEXITERATOR_TYPE(start_leaf, 0, buffer_pool_manager_);
    }

/*
 * Input parameter is low key, find the leaf page that contains the input key
 * first, then construct index iterator
 * @return : index iterator
 */
    INDEX_TEMPLATE_ARGUMENTS
    INDEXITERATOR_TYPE BPLUSTREE_TYPE::Begin(const KeyType &key) {
        ////寻找index
        auto start_leaf = FindLeafPage(key); ////先通过key找到leaf page
        TryUnlockRootPageId(false);
        if (start_leaf == nullptr) { ////没找到，则返回0
            return INDEXITERATOR_TYPE(start_leaf, 0, buffer_pool_manager_);
        }
        int idx = start_leaf->KeyIndex(key,comparator_); ////找到了，构造idx的index iterator
        return INDEXITERATOR_TYPE(start_leaf, idx, buffer_pool_manager_);//return
    }

/*****************************************************************************
 * UTILITIES AND DEBUG
 *****************************************************************************/
/*
 * Find leaf page containing particular key, if leftMost flag == true, find
 * the left most leaf page
 */
    INDEX_TEMPLATE_ARGUMENTS
    B_PLUS_TREE_LEAF_PAGE_TYPE *BPLUSTREE_TYPE::FindLeafPage(const KeyType &key,
                                                             bool leftMost,eOpType op,
                                                             Transaction *transaction) {
        bool exclusive = (op != eOpType::READ);
        LockRootPageId(exclusive); ////首先上锁，防止多线程问题
        if (IsEmpty()) {
            TryUnlockRootPageId(exclusive);
            return nullptr;
        }
        auto pointer = CrabingProtocalFetchPage(root_page_id_,op,-1,transaction);
        page_id_t next;
        for (page_id_t cur = root_page_id_;
            !pointer->IsLeafPage();
            pointer = CrabingProtocalFetchPage(next,op,cur,transaction),cur = next) {
            ////for 遍历
            auto *internalPage = static_cast<B_PLUS_TREE_INTERNAL_PAGE *>(pointer);
            if (leftMost) {
                next = internalPage->ValueAt(0);
            }else {
                next = internalPage->Lookup(key,comparator_);
            }
        }
        return static_cast<B_PLUS_TREE_LEAF_PAGE_TYPE *>(pointer);
    }
    INDEX_TEMPLATE_ARGUMENTS
    BPlusTreePage *BPLUSTREE_TYPE::FetchPage(page_id_t page_id) {
        auto page = buffer_pool_manager_->FetchPage(page_id);
        return reinterpret_cast<BPlusTreePage *>(page->GetData());
    }
    INDEX_TEMPLATE_ARGUMENTS
    BPlusTreePage *BPLUSTREE_TYPE::CrabingProtocalFetchPage(page_id_t page_id,eOpType op,page_id_t previous, Transaction *transaction) {
        bool exclusive = op != eOpType::READ;
        auto page = buffer_pool_manager_->FetchPage(page_id);////获取page
        Lock(exclusive,page);
        auto tree_page = reinterpret_cast<BPlusTreePage *>(page->GetData());
        if (previous > 0 && (!exclusive || tree_page->isSafe(op))) {
            FreePagesInTransaction(exclusive,transaction,previous);
        }
        if (transaction != nullptr)
            transaction->AddIntoPageSet(page);
        return tree_page;
    }

    INDEX_TEMPLATE_ARGUMENTS
    void BPLUSTREE_TYPE::FreePagesInTransaction(bool exclusive, Transaction *transaction, page_id_t cur) {
        TryUnlockRootPageId(exclusive);
        if (transaction == nullptr) {////为空则不进行操作
            assert(!exclusive && cur >= 0);
            Unlock(false,cur);
            buffer_pool_manager_->UnpinPage(cur,false);
            return;
        }
        ////transaction不为空
        for (Page *page : *transaction->GetPageSet()) {
            int cur_pid = page->GetPageId();
            Unlock(exclusive,page);
            buffer_pool_manager_->UnpinPage(cur_pid,exclusive);
            ////找到了tar page
            if (transaction->GetDeletedPageSet()->find(cur_pid) != transaction->GetDeletedPageSet()->end()) {
                buffer_pool_manager_->DeletePage(cur_pid);////则删除
                transaction->GetDeletedPageSet()->erase(cur_pid);
            }
        }
        assert(transaction->GetDeletedPageSet()->empty());
        transaction->GetPageSet()->clear();
    }

/*
 * Update/Insert root page id in header page(where page_id = 0, header_page is
 * defined under include/page/header_page.h)
 * Call this method everytime root page id is changed.
 * @parameter: insert_record      defualt value is false. When set to true,
 * insert a record <index_name, root_page_id> into header page instead of
 * updating it.
 */
    INDEX_TEMPLATE_ARGUMENTS
    void BPLUSTREE_TYPE::UpdateRootPageId(int insert_record) {
        auto *header_page = static_cast<HeaderPage *>(
                buffer_pool_manager_->FetchPage(HEADER_PAGE_ID));
        if (insert_record){////需要插入一条新记录
            ////在header_page中创建新记录<index_name+root_page_id>
            header_page->InsertRecord(index_name_, root_page_id_);
        } else{////一般为false
            ////更新header_page中的root_page_id
            header_page->UpdateRecord(index_name_, root_page_id_);
        }
        buffer_pool_manager_->UnpinPage(HEADER_PAGE_ID, true);
    }

/*
 * This method is used for debug only
 * print out whole b+tree sturcture, rank by rank
 */
    INDEX_TEMPLATE_ARGUMENTS
    std::string BPLUSTREE_TYPE::ToString(bool verbose) {
        if (IsEmpty()) {
            return "Empty tree";
        }
        std::queue<BPlusTreePage *> todo, tmp;
        std::stringstream tree;
        auto node = reinterpret_cast<BPlusTreePage *>(
                buffer_pool_manager_->FetchPage(root_page_id_));
        if (node == nullptr) {
            throw Exception(EXCEPTION_TYPE_INDEX,
                            "all page are pinned while printing");
        }
        todo.push(node);
        bool first = true;
        while (!todo.empty()) {
            node = todo.front();
            if (first) {
                first = false;
                tree << "| ";
            }
            // leaf page, print all key-value pairs
            if (node->IsLeafPage()) {
                auto page = reinterpret_cast<BPlusTreeLeafPage<KeyType, ValueType, KeyComparator> *>(node);
                tree << page->ToString(verbose) <<"("<<node->GetPageId()<< ")| ";
            } else {
                auto page = reinterpret_cast<BPlusTreeInternalPage<KeyType, page_id_t, KeyComparator> *>(node);
                tree << page->ToString(verbose) <<"("<<node->GetPageId()<< ")| ";
                page->QueueUpChildren(&tmp, buffer_pool_manager_);
            }
            todo.pop();
            if (todo.empty() && !tmp.empty()) {
                todo.swap(tmp);
                tree << '\n';
                first = true;
            }
            // unpin node when we are done
            buffer_pool_manager_->UnpinPage(node->GetPageId(), false);
        }
        return tree.str();
    }

/*
 * This method is used for test only
 * Read data from file and insert one by one
 */
    INDEX_TEMPLATE_ARGUMENTS
    void BPLUSTREE_TYPE::InsertFromFile(const std::string &file_name,
                                        Transaction *transaction) {
        int64_t key;
        std::ifstream input(file_name);
        while (input) {///遍历文件
            input >> key;
            KeyType index_key;
            index_key.SetFromInteger(key);
            RID rid(key);
            Insert(index_key, rid, transaction);
        }
    }
/*
 * This method is used for test only
 * Read data from file and remove one by one
 */
    INDEX_TEMPLATE_ARGUMENTS
    void BPLUSTREE_TYPE::RemoveFromFile(const std::string &file_name,
                                        Transaction *transaction) {
        int64_t key;
        std::ifstream input(file_name);
        while (input) {
            input >> key;
            KeyType index_key;
            index_key.SetFromInteger(key);
            Remove(index_key, transaction);
        }
    }


/***************************************************************************
 *  Check integrity of B+ tree data structure.
 ***************************************************************************/

    ////判断是否是平衡树
    INDEX_TEMPLATE_ARGUMENTS
    int BPLUSTREE_TYPE::isBalanced(page_id_t pid) {
        if (IsEmpty()) return true;
        ////通过pid获取目标page
        auto node = reinterpret_cast<BPlusTreePage *>(buffer_pool_manager_->FetchPage(pid));
        int ret = 0;
        if (!node->IsLeafPage())  {////不是叶子节点，则进行向下走判断
            ////得到节点的page
            auto page = reinterpret_cast<B_PLUS_TREE_INTERNAL_PAGE *>(node);
            int last = -2;
            ////遍历此page
            for (int i = 0; i < page->GetSize(); i++) {
                //递归判断
                int cur = isBalanced(page->ValueAt(i));
                if (cur >= 0 && last == -2) {
                    last = cur;
                    ret = last + 1;
                }else if (last != cur) {
                    ret = -1;
                    break;
                }
            }
        }
        ////解除封锁
        buffer_pool_manager_->UnpinPage(pid,false);
        return ret;
    }

    INDEX_TEMPLATE_ARGUMENTS
    bool BPLUSTREE_TYPE::isPageCorr(page_id_t pid,pair<KeyType,KeyType> &out) {
        if (IsEmpty()) return true;
        else{
            auto node = reinterpret_cast<BPlusTreePage *>(buffer_pool_manager_->FetchPage(pid));
            if (node == nullptr) {
                throw Exception(EXCEPTION_TYPE_INDEX,"all page are pinned while isPageCorr");
            }
            bool ret = true;
            if (node->IsLeafPage())  {
                auto page = reinterpret_cast<BPlusTreeLeafPage<KeyType, ValueType, KeyComparator> *>(node);
                int size = page->GetSize();
                ret = ret && (size >= node->GetMinSize() && size <= node->GetMaxSize());
                for (int i = 1; i < size; i++) {
                    if (comparator_(page->KeyAt(i-1), page->KeyAt(i)) > 0) {
                        ret = false;
                        break;
                    }
                }
                out = pair<KeyType,KeyType>{page->KeyAt(0),page->KeyAt(size-1)};
            } else {
                auto page = reinterpret_cast<BPlusTreeInternalPage<KeyType, page_id_t, KeyComparator> *>(node);
                int size = page->GetSize();
                ret = ret && (size >= node->GetMinSize() && size <= node->GetMaxSize());
                pair<KeyType,KeyType> left,right;
                for (int i = 1; i < size; i++) {
                    if (i == 1) {
                        ret = ret && isPageCorr(page->ValueAt(0),left);
                    }
                    ret = ret && isPageCorr(page->ValueAt(i),right);
                    ret = ret && (comparator_(page->KeyAt(i) ,left.second)>0 && comparator_(page->KeyAt(i), right.first)<=0);
                    ret = ret && (i == 1 || comparator_(page->KeyAt(i-1) , page->KeyAt(i)) < 0);
                    if (!ret) break;
                    left = right;
                }
                out = pair<KeyType,KeyType>{page->KeyAt(0),page->KeyAt(size-1)};
            }
            buffer_pool_manager_->UnpinPage(pid,false);
            return ret;
        }

    }

    ////my private function
    INDEX_TEMPLATE_ARGUMENTS
    bool BPLUSTREE_TYPE::Check(bool forceCheck) {
        if (!forceCheck && !openCheck) {
            return true;
        }
        pair<KeyType,KeyType> in;
        bool isPageInOrderAndSizeCorr = isPageCorr(root_page_id_, in);
        bool isBal = (isBalanced(root_page_id_) >= 0);
        bool isAllUnpin = buffer_pool_manager_->CheckAllUnpined();
        if (!isPageInOrderAndSizeCorr) cout<<"problem in page order or page size"<<endl;
        if (!isBal) cout<<"problem in balance"<<endl;
        if (!isAllUnpin) cout<<"problem in page unpin"<<endl;
        return isPageInOrderAndSizeCorr && isBal && isAllUnpin;
    }

    template <typename KeyType, typename ValueType, typename KeyComparator>
    thread_local int BPlusTree<KeyType, ValueType, KeyComparator>::mRootLockedCnt = 0;

    ////my private helper function begin
    ////|||实现lock和unlock来进行封锁，保证多用户环境下并发的正确性
    template<typename KeyType, typename ValueType, typename KeyComparator>
    void BPlusTree<KeyType, ValueType, KeyComparator>::Lock(bool exclusive, Page *page) {
        if (exclusive) {////排他锁说明是写锁，独占数据
            page->WLatch();
        } else {
            page->RLatch();
        }
    }

    ////my private helper function
    template<typename KeyType, typename ValueType, typename KeyComparator>
    void BPlusTree<KeyType, ValueType, KeyComparator>::Unlock(bool exclusive, Page *page) {
        if (exclusive) {////排他锁说明是写锁，独占数据
            page->WUnlatch();
        } else {
            page->RUnlatch();
        }
    }

    ////my private helper function
    ////解锁
    template<typename KeyType, typename ValueType, typename KeyComparator>
    void BPlusTree<KeyType, ValueType, KeyComparator>::Unlock(bool exclusive, page_id_t pageId) {
        auto page = buffer_pool_manager_->FetchPage(pageId);////拉起，保护page
        Unlock(exclusive,page);//解锁
        buffer_pool_manager_->UnpinPage(pageId,exclusive);////在释放

    }

    ////my private helper function
    template<typename KeyType, typename ValueType, typename KeyComparator>
    void BPlusTree<KeyType, ValueType, KeyComparator>::LockRootPageId(bool exclusive) {
        if (exclusive) {
            mMutex_.WLock();
        } else {
            mMutex_.RLock();
        }
        mRootLockedCnt++;
    }

    ////my private helper function
    template<typename KeyType, typename ValueType, typename KeyComparator>
    void BPlusTree<KeyType, ValueType, KeyComparator>::TryUnlockRootPageId(bool exclusive) {
        if (mRootLockedCnt > 0) {
            if (exclusive) {////独占锁，写锁
                mMutex_.WUnlock();
            } else {
                mMutex_.RUnlock();
            }
            mRootLockedCnt--;
        }
    }


    template class BPlusTree<GenericKey<4>, RID, GenericComparator<4>>;
    template class BPlusTree<GenericKey<8>, RID, GenericComparator<8>>;
    template class BPlusTree<GenericKey<16>, RID, GenericComparator<16>>;
    template class BPlusTree<GenericKey<32>, RID, GenericComparator<32>>;
    template class BPlusTree<GenericKey<64>, RID, GenericComparator<64>>;
} // namespace scudb
