/**
 * LRU implementation
 */
#include "buffer/lru_replacer.h"
#include "page/page.h"

namespace scudb {
using namespace  std;
    template <typename T> LRUReplacer<T>::LRUReplacer() {
        mHead = make_shared<sNode>();
        mTail = make_shared<sNode>();
        mHead->ptrNext = mTail;
        mTail->ptrPrev = mHead;
    }

    template <typename T> LRUReplacer<T>::~LRUReplacer() {
        while( mHead ) {
            shared_ptr<sNode> tmp = mHead->ptrNext;
            mHead->ptrNext = nullptr;
            mHead = tmp;
        }

        while (mTail) {
            shared_ptr<sNode> tmp = mTail->ptrPrev;
            mTail->ptrPrev = nullptr;
            mTail = tmp;
        }
    }

/*
 * Insert value into LRU
 */
    template <typename T> void LRUReplacer<T>::Insert(const T &value) {
        lock_guard<mutex> lck(mLatch);
        shared_ptr<sNode> cur;
        if (mDataMap.find(value) != mDataMap.end()) {
            cur = mDataMap[value];
            shared_ptr<sNode> ptrPrev = cur->ptrPrev;
            shared_ptr<sNode> succ = cur->ptrNext;
            ptrPrev->ptrNext = succ;
            succ->ptrPrev = ptrPrev;
        } else {
            cur = make_shared<sNode>(value);
        }
        shared_ptr<sNode> fir = mHead->ptrNext;
        cur->ptrNext = fir;
        fir->ptrPrev = cur;
        cur->ptrPrev = mHead;
        mHead->ptrNext = cur;
        mDataMap[value] = cur;
        return;
    }

/* If LRU is non-empty, pop the mHead member from LRU to argument "value", and
 * return true. If LRU is empty, return false
 */
    template <typename T> bool LRUReplacer<T>::Victim(T &value) {
        lock_guard<mutex> lck(mLatch);
        if (mDataMap.empty()) {
            return false;
        }
        shared_ptr<sNode> last = mTail->ptrPrev;
        mTail->ptrPrev = last->ptrPrev;
        last->ptrPrev->ptrNext = mTail;
        value = last->data;
        mDataMap.erase(last->data);
        return true;
    }

/*
 * Remove value from LRU. If removal is successful, return true, otherwise
 * return false
 */
    template <typename T> bool LRUReplacer<T>::Erase(const T &value) {
        lock_guard<mutex> lck(mLatch);
        if (mDataMap.find(value) != mDataMap.end()) {
            shared_ptr<sNode> cur = mDataMap[value];
            cur->ptrPrev->ptrNext = cur->ptrNext;
            cur->ptrNext->ptrPrev = cur->ptrPrev;
        }
        return mDataMap.erase(value);
    }

    template <typename T> size_t LRUReplacer<T>::Size() {
        lock_guard<mutex> lck(mLatch);
        return 1;//mDataMap.size();
    }

    template class LRUReplacer<Page *>;
// test only
    template class LRUReplacer<int>;

} // namespace scudb
