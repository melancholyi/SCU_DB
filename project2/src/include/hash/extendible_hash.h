/*
 * extendible_hash.h : implementation of in-memory hash table using extendible
 * hashing
 *
 * Functionality: The buffer pool manager must maintain a page table to be able
 * to quickly map a PageId to its corresponding memory location; or alternately
 * report that the PageId does not match any currently-buffered page.
 */

#pragma once

#include <cstdlib>
#include <vector>
#include <string>

#include "hash/hash_table.h"

#include <map>
#include <memory>
#include <mutex>




namespace scudb {


template <typename K, typename V>
class ExtendibleHash : public HashTable<K, V> {
    struct sBucket {
        sBucket(int depth) : localDepth(depth) {};
        int localDepth;
        std::map<K, V> dataMap;
        std::mutex latch;
    };



public:

    // constructor
    explicit ExtendibleHash(size_t size);
    explicit ExtendibleHash();
    // helper function to generate hash addressing
    size_t HashKey(const K &key) const;

    // helper function to get global & local depth
    int GetGlobalDepth() const;
    int GetLocalDepth(int bucket_id) const;

    int GetNumBuckets() const;
    // lookup and modifier
    bool Find(const K &key, V &value) override;
    bool Remove(const K &key) override;
    void Insert(const K &key,const V &value) override;

private:
    int getBucketIndex(const K &key) const;

private:
    std::vector<std::shared_ptr<sBucket>> mBuckets;  //buckets vector
    mutable std::mutex mLatch;    //latch membership
    int mGlobalDepth;       //global depth
    size_t mBucketSize;     //each bucket size
    int mBucketNum;         //bucket all num

};
} // namespace scudb
