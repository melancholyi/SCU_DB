/**
 * lru_replacer.h
 *
 * Functionality: The buffer pool manager must maintain a LRU list to collect
 * all the pages that are unpinned and ready to be swapped. The simplest way to
 * implement LRU is a FIFO queue, but remember to dequeue or enqueue pages when
 * a page changes from unpinned to pinned, or vice-versa.
 */

#pragma once

#include "buffer/replacer.h"
#include "hash/extendible_hash.h"

#include <memory>
#include <mutex>
#include <unordered_map>

namespace scudb {

template <typename T> class LRUReplacer : public Replacer<T> {
private:
    //node of double list
    struct sNode{
        //default constructor
        sNode() {data = 0 ; ptrPrev = nullptr; ptrNext = nullptr;}
        sNode(T data) : data(data){};
        T data;
        std::shared_ptr<sNode> ptrPrev;
        std::shared_ptr<sNode> ptrNext;
    };

public:
  // do not change public interface
  LRUReplacer();

  ~LRUReplacer();

  void Insert(const T &value);

  bool Victim(T &value);

  bool Erase(const T &value);

  size_t Size();

private:
  // add your member variables here
  std::shared_ptr<sNode> mHead;
  std::shared_ptr<sNode> mTail;
  std::unordered_map<T,std::shared_ptr<sNode>> mDataMap;
  mutable std::mutex mLatch;
};

} // namespace scudb
