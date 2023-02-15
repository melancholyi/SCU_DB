#include <list>

#include "hash/extendible_hash.h"
#include "page/page.h"

namespace scudb {

/*
 * constructor
 * array_size: fixed array size for each bucket
 */
template <typename K, typename V>
ExtendibleHash<K, V>::ExtendibleHash(size_t size) :
    mGlobalDepth(0),   //
    mBucketSize(size), // fixed array size for each bucket
    mBucketNum(1) {    // bucket default num = 1
        mBuckets.push_back(std::make_shared<sBucket>(0)); //local depth default = 0
}


/*
 * helper function to calculate the hashing address of input key
 */
template <typename K, typename V>
size_t ExtendibleHash<K, V>::HashKey(const K &key) const{
    return std::hash<K>{}(key);
}

/*
 * helper function to return global depth of hash table
 * NOTE: you must implement this function in order to pass test
 */
template <typename K, typename V>
int ExtendibleHash<K, V>::GetGlobalDepth() const{
    std::lock_guard<std::mutex> lock(mLatch);
    return mGlobalDepth;
}

/*
 * helper function to return local depth of one specific bucket
 * NOTE: you must implement this function in order to pass test
 */
template <typename K, typename V>
int ExtendibleHash<K, V>::GetLocalDepth(int bucket_id) const {
    //lock_guard<mutex> lck2(latch);
    if (mBuckets[bucket_id]) {   // ptr is not nullptr
        std::lock_guard<std::mutex> lock(mBuckets[bucket_id]->latch); //latch mutex
        if (mBuckets[bucket_id]->dataMap.size() == 0) {
            return -1; //no data return -1
        }else{
            return mBuckets[bucket_id]->localDepth; // normal  return
        };
    }
    else{
        return -1; //
    }
}

/*
 * helper function to return current number of bucket in hash table
 */
template <typename K, typename V>
int ExtendibleHash<K, V>::GetNumBuckets() const{ //bucket num in hash table
    std::lock_guard<std::mutex> lock(mLatch); //
    return mBucketNum;
}

/*
 * lookup function to find value associate with input key
 */
template <typename K, typename V>
bool ExtendibleHash<K, V>::Find(const K &key, V &value) {
    int index = getBucketIndex(key);
    // std::lock_guard<std::mutex> lck(mBuckets[idx]->latch); //lock
    if (mBuckets[index]->dataMap.find(key) != mBuckets[index]->dataMap.end()) { //map find ok
        value = mBuckets[index]->dataMap[key]; //set value
        return true;
    }else{
        return false;
    }
}


template <typename K, typename V>
int ExtendibleHash<K, V>::getBucketIndex(const K &key) const{
    std::lock_guard<std::mutex> lck(mLatch);
    return HashKey(key) & ((1 << mGlobalDepth) - 1);
}

/*
 * delete <key,value> entry in hash table
 * Shrink & Combination is not required for this project
 */
template <typename K, typename V>
bool ExtendibleHash<K, V>::Remove(const K &key) {
    int index = getBucketIndex(key); // index
    std::lock_guard<std::mutex> lock(mBuckets[index]->latch); //lock
    if (mBuckets[index]->dataMap.find(key) == mBuckets[index]->dataMap.end()) {
        return false;
    }else{
        mBuckets[index]->dataMap.erase(key);
        return true;
    }
}

/*
 * insert <key,value> entry in hash table
 * Split & Redistribute bucket when there is overflow and if necessary increase
 * global depth
 */
template <typename K, typename V>
void ExtendibleHash<K, V>::Insert(const K &key, const V &value) {
    int index = getBucketIndex(key); //find index
    std::shared_ptr<sBucket> cur = mBuckets[index]; //temp bucket ptr

    while (true) {
        std::lock_guard<std::mutex> lock(cur->latch); //lock bucket latch

        //dataMap have location
        if (cur->dataMap.find(key) != cur->dataMap.end() || cur->dataMap.size() < mBucketSize) {
            cur->dataMap.insert(std::make_pair(key,value)); // insert pair
            break;
        }

        int mask = (1 << (cur->localDepth)); // mask
        cur->localDepth++; //local Depth ++

        // local code segment
        {
            std::lock_guard<std::mutex> lock2(mLatch);
            if (cur->localDepth > mGlobalDepth) { //overflow
                size_t length = mBuckets.size();
                for (size_t i = 0; i < length; i++) {
                    mBuckets.push_back(mBuckets[i]);
                }
                mGlobalDepth++;
            }
            mBucketNum++;
            auto newBuc = std::make_shared<sBucket>(cur->localDepth);

            for (auto it = cur->dataMap.begin(); it != cur->dataMap.end(); ) {
                if (HashKey(it->first) & mask) {
                    newBuc->dataMap[it->first] = it->second;
                    it = cur->dataMap.erase(it);
                } else it++;
            }
            for (size_t i = 0; i < mBuckets.size(); i++) {
                if (mBuckets[i] == cur && (i & mask))
                    mBuckets[i] = newBuc;
            }
        }

        index = getBucketIndex(key);
        cur = mBuckets[index];
    }
}

template<typename K, typename V>
ExtendibleHash<K, V>::ExtendibleHash() {
    ExtendibleHash(64);
}

template class ExtendibleHash<page_id_t, Page *>;
template class ExtendibleHash<Page *, std::list<Page *>::iterator>;

//// test purpose
template class ExtendibleHash<int, std::string>;
template class ExtendibleHash<int, std::list<int>::iterator>;
template class ExtendibleHash<int, int>;
} // namespace scudb
