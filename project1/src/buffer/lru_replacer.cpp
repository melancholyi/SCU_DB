/**
 * LRU implementation
 */
#include "buffer/lru_replacer.h"
#include "page/page.h"

namespace scudb {
    /**
     * constructor
     * @tparam T
     */
template <typename T> LRUReplacer<T>::LRUReplacer() {
    mHead = std::make_shared<sNode>();
    mTail = std::make_shared<sNode>();
    mHead->ptrNext = mTail;
    mTail->ptrPrev = mHead;
}

template <typename T> LRUReplacer<T>::~LRUReplacer() {}

/*
 * Insert value into LRU
 */
template <typename T> void LRUReplacer<T>::Insert(const T &value) {
    ////lock mutex
    std::lock_guard<std::mutex> lock(mLatch);

    ////get the node ptr of value
    std::shared_ptr<sNode> ptrNow;
    if(mDataMap.find(value) != mDataMap.end()){
        ////find value, change its position in double list
        ptrNow = mDataMap[value];
        ////double list ptr concat
        ptrNow->ptrPrev->ptrNext = ptrNow->ptrNext;
        ptrNow->ptrNext->ptrPrev = ptrNow->ptrPrev;
    }
    else{
        ptrNow = std::make_shared<sNode>(value);
    }

    std::shared_ptr<sNode> first = mHead->ptrNext;

    ////set the node of value insert first node
    //// CONCAT THE FIRST NODE
    ptrNow->ptrNext = first;
    first->ptrPrev = ptrNow;

    //// concat the head node
    ptrNow->ptrPrev = mHead;
    mHead->ptrNext = ptrNow;

    ////reset the node pos of value
    mDataMap[value] = ptrNow;
}

/* If LRU is non-empty, pop the head member from LRU to argument "value", and
 * return true. If LRU is empty, return false
 */
template <typename T> bool LRUReplacer<T>::Victim(T &value) {
    std::lock_guard<std::mutex> lock(mLatch);
    if(mDataMap.empty()){
        return false;
    }else{
        ////choose the last node to victim
        //change ptr
        std::shared_ptr<sNode> ptrLast = mTail->ptrPrev;
        mTail->ptrPrev  = ptrLast->ptrPrev;
        ptrLast->ptrPrev->ptrNext = mTail;
        value = ptrLast->data; //return value
        mDataMap.erase(ptrLast->data); //eraser
        return true;
    }
}

/*
 * Remove value from LRU. If removal is successful, return true, otherwise
 * return false
 */
template <typename T> bool LRUReplacer<T>::Erase(const T &value) {
    std::lock_guard<std::mutex> lock(mLatch);

    ////is find?
    if(mDataMap.find(value) != mDataMap.end()){
        std::shared_ptr<sNode> now = mDataMap[value];
        std::shared_ptr<sNode> prev = now->ptrPrev,next = now->ptrNext;
        ////ptr update
        prev->ptrNext = now->ptrNext;
        next->ptrPrev = now->ptrPrev;
    }
    ////erase
    if(mDataMap.erase(value)){
        return true;
    }else{
        return false;
    }
}

template <typename T> size_t LRUReplacer<T>::Size() {
    std::lock_guard<std::mutex> lock (mLatch);
    return mDataMap.size();
}

template class LRUReplacer<Page *>;
// test only
template class LRUReplacer<int>;

} // namespace scudb
