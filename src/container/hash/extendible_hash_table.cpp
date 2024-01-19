//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// extendible_hash_table.cpp
//
// Identification: src/container/hash/extendible_hash_table.cpp
//
// Copyright (c) 2022, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include <cassert>
#include <cstdlib>
#include <functional>
#include <list>
#include <utility>

#include "container/hash/extendible_hash_table.h"
#include "storage/page/page.h"

#include "common/logger.h"

namespace bustub {

template <typename K, typename V>
ExtendibleHashTable<K, V>::ExtendibleHashTable(size_t bucket_size)
    : global_depth_(0), bucket_size_(bucket_size), num_buckets_(1) {
  // 还要补充点东西!!!dir_里面应该初始化为一层，对应一个桶要放进去

  dir_.push_back(std::shared_ptr<Bucket>(new Bucket(bucket_size_, 0)));
  // 这里pb的内容有点乱，看cpptest文件里面初始化那里的整理
}
// 初始化全局深度是0,桶的个数一开始也是1

// p1开始比较正规，p0不正规，类中函数实现在h文件
// .h 文件都是在声明和引入，宏定义等内容，具体实现都是放在cpp文件里面，就好像这个文件,用::域运算符实现类中函数

template <typename K, typename V>
auto ExtendibleHashTable<K, V>::IndexOf(const K &key) -> size_t {
  int mask = (1 << global_depth_) - 1;
  return std::hash<K>()(key) & mask;
  // hash函数，和每次取后几位（取决于depth）
  // 01010   10 取出来的位
}

template <typename K, typename V>
auto ExtendibleHashTable<K, V>::GetGlobalDepth() const -> int {
  std::scoped_lock<std::mutex> lock(latch_);
  return GetGlobalDepthInternal();
}

template <typename K, typename V>
auto ExtendibleHashTable<K, V>::GetGlobalDepthInternal() const -> int {
  return global_depth_;
}

template <typename K, typename V>
auto ExtendibleHashTable<K, V>::GetLocalDepth(int dir_index) const -> int {
  std::scoped_lock<std::mutex> lock(latch_);
  return GetLocalDepthInternal(dir_index);
}

template <typename K, typename V>
auto ExtendibleHashTable<K, V>::GetLocalDepthInternal(int dir_index) const -> int {
  return dir_[dir_index]->GetDepth();
}

template <typename K, typename V>
auto ExtendibleHashTable<K, V>::GetNumBuckets() const -> int {
  std::scoped_lock<std::mutex> lock(latch_);
  return GetNumBucketsInternal();
}

template <typename K, typename V>
auto ExtendibleHashTable<K, V>::GetNumBucketsInternal() const -> int {
  return num_buckets_;
}

template <typename K, typename V>
auto ExtendibleHashTable<K, V>::Find(const K &key, V &value) -> bool {
  // std::scoped_lock<std::mutex> lock(latch_);
  // 我抄的答案这里直接用scoped_lock管理锁
  // 这个和智能指针一样，基于RAII思想，离开scoped_lock作用域之后，能够自行的解锁
  // 防止产生死锁，但是我这里就读前面上锁，读后解锁，操作简单，可以不用
  // 分支条件多了，容易乱，就是和用这个
  // latch_.lock();
  std::scoped_lock<std::mutex> lock(latch_);
  auto res = dir_[IndexOf(key)]->Find(key, value);
  // latch_.unlock();
  return res;
  // UNREACHABLE("not implemented");
  // 这个就是define后，用来没实现这个函数报错用的，提示哪些函数需要实现
}

template <typename K, typename V>
auto ExtendibleHashTable<K, V>::Remove(const K &key) -> bool {
  // latch_.lock();
  std::scoped_lock<std::mutex> lock(latch_);
  auto res = dir_[IndexOf(key)]->Remove(key);
  // dir_[IndexOf(key)] 对象类型是std::shared_ptr<Bucket>，可以直接当成是一个bucket来用，无视这个智能指针，就当是
  // *p->函数or变量 bucket 里面已经有判断不存在的情况了，所以这里不需要再find判断有没有了
  // latch_.unlock();
  return res;
  // remove和find函数都是简单函数，hash找到桶，对桶进行对应操作即可
  // insert和重构桶才是task1的核心任务
  // UNREACHABLE("not implemented");
}

template <typename K, typename V>
void ExtendibleHashTable<K, V>::RedistributeBucket(std::shared_ptr<Bucket> bucket) {
  // ！！！桶满之后的重新分配，核心代码！！！
  // 全部自己实现的，其实挺简单的
  bucket->IncrementDepth();
  num_buckets_++;
  auto checkpos = bucket->GetDepth() - 1;
  std::shared_ptr<Bucket> new_bucket(new Bucket(bucket_size_, bucket->GetDepth()));
  auto list = bucket->GetItems();  // 桶里的东西先拿出来
  // 先把不知道多少个指向原先这个桶的dir_ 分别指向对应旧桶和新桶
  // 这个简单点的方法，直接遍历整个dir，想省时间，可以多传一个参数，把桶dir_[id]
  // id传进来，这样，直接之后后面几位是什么，然后直接对高位进行枚举，
  // 但这样写起来及其麻烦，以后追求性能这里可以改一改
  for (int i = 0; i < (1 << global_depth_); i++) {
    if (dir_[i] == bucket) {
      int nex = (i >> checkpos) & 1;
      if (nex == 1) {
        dir_[i] = new_bucket;
      }
    }
  }
  // 下一位要是是1，那就把他指向新桶，否则，指向旧桶(不动即可)

  // LOG_INFO("%d",(int)bucket->IsFull());
  // 经过测试，这样只是把原来list拿了一份复制，不能清空原有桶，清空桶没给函数
  // 桶标号对应完后，只需要把前面桶里面东西rehash一下就行了。
  // 两个方法，一个是原来的桶清空，然后东西都再跑一遍，但是这个方法要加一个桶清空函数
  // 我这里选择了先rehash，要是在原来那个桶里面，就不动，否则，从原来那个桶里面remove掉，然后insert到新桶里面
  for (auto &[k, v] : list) {
    // dir_[IndexOf(k)]->Insert(k,v);
    auto tobucket = dir_[IndexOf(k)];
    if (tobucket != bucket) {
      bucket->Remove(k);
      tobucket->Insert(k, v);
    }
  }
}

template <typename K, typename V>
void ExtendibleHashTable<K, V>::Insert(const K &key, const V &value) {
  // latch_.lock();
  std::scoped_lock<std::mutex> lock(latch_);
  while (true) {
    auto bucket_id = IndexOf(key);
    // 这里我的命名和另外一个获取桶的local深度那个函数里面的传入的dir_index是一个东西
    // 就不改了
    auto i_bucket = dir_[bucket_id];  // 需要插入的桶的指针
    if (i_bucket->Insert(key, value)) {
      // 有空间插入成功才会break
      break;
    }
    if (i_bucket->GetDepth() == global_depth_) {  // dir分裂，全局深度+1
      // 这里读全局深度直接读，因为函数那里被上锁了，会获取两遍锁
      // 桶的局部深度也是一个原因
      auto temp = dir_;
      global_depth_++;
      // 全局深度+1,但是他居然没给对应操作函数，可能直接类里面不需要，而桶的话是在类外，需要对应操作函数
      for (auto &item : temp) {
        dir_.push_back(item);
      }
    }  // dir_增加一层，直接copy一遍dir_内容即可，因为最高位不看，剩下几位相当于一个内容一样的状态
    // 不管分裂完成没，这个桶还是满的，所以还是要重新分布这个桶
    RedistributeBucket(i_bucket);
  }  // 因为重新分布之后，可能还是有这个桶任然是满的，所以死循环跑死
  // UNREACHABLE("not implemented");
}

//===--------------------------------------------------------------------===//
// Bucket
//===--------------------------------------------------------------------===//
template <typename K, typename V>
ExtendibleHashTable<K, V>::Bucket::Bucket(size_t array_size, int depth) : size_(array_size), depth_(depth) {}
// 构造函数，不用实现，初始化已经够了

template <typename K, typename V>
auto ExtendibleHashTable<K, V>::Bucket::Find(const K &key, V &value) -> bool {
  // list，底层是带头的双向链表，没有自己的find 函数，所以需要自己on遍历去实现：
  // 当然，用全局的 find可以使可以实现，复杂度应该也是on吧，也是遍历一遍
  for (auto &[k, v] : list_) {
    if (k == key) {
      value = v;
      return true;
    }
  }
  return false;
  // UNREACHABLE("not implemented");
}

template <typename K, typename V>
auto ExtendibleHashTable<K, V>::Bucket::Remove(const K &key) -> bool {
  auto it = list_.begin();
  // remove(值) erase(迭代器)
  while (it != list_.end()) {
    if ((*it).first == key) {
      list_.erase(it);
      // 要是继续遍历，就得it++
      return true;
    }
    it++;
  }

  return false;
  // UNREACHABLE("not implemented");
}

template <typename K, typename V>
auto ExtendibleHashTable<K, V>::Bucket::Insert(const K &key, const V &value) -> bool {
  // 这里遍历要&取引用，因为需要对list_里面存在的值进行修改了！！！
  for (auto &[k, v] : list_) {
    if (k == key) {
      v = value;
      return true;
    }
  }
  // 要先查是否已有，再判断是否满，因为满判断是因为没有要插入了，才判断的
  if (IsFull()) {
    return false;
  }

  // list_.push_back({key,value});
  list_.push_back(std::make_pair(key, value));
  return true;
  // UNREACHABLE("not implemented");
}

// 这几行是干什么的？删除之后还能用吗？：
// 这个不是具体模板套上了具体类型吗？？？？
template class ExtendibleHashTable<page_id_t, Page *>;
template class ExtendibleHashTable<Page *, std::list<Page *>::iterator>;
template class ExtendibleHashTable<int, int>;
// test purpose
template class ExtendibleHashTable<int, std::string>;
template class ExtendibleHashTable<int, std::list<int>::iterator>;

}  // namespace bustub
