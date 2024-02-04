//===----------------------------------------------------------------------===//
//
//                         CMU-DB Project (15-445/645)
//                         ***DO NO SHARE PUBLICLY***
//
// Identification: src/page/b_plus_tree_leaf_page.cpp
//
// Copyright (c) 2018, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include <sstream>

#include "common/exception.h"
#include "common/rid.h"
#include "storage/page/b_plus_tree_leaf_page.h"

namespace bustub {

/*****************************************************************************
 * HELPER METHODS AND UTILITIES
 *****************************************************************************/

/**
 * Init method after creating a new leaf page
 * Including set page type, set current size to zero, set page id/parent id, set
 * next page id and set max size
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_LEAF_PAGE_TYPE::Init(page_id_t page_id, page_id_t parent_id, int max_size) {
  SetPageType(IndexPageType::LEAF_PAGE);
  SetSize(0);  // 这里明确说了要为0
  SetPageId(page_id);
  SetParentPageId(parent_id);
  SetMaxSize(max_size);
  SetNextPageId(INVALID_PAGE_ID);

  // 还是一样，先加上在说
  SetLSN();
}
// 这里size0是因为叶子结点，不需要关键字和对应数据结点之间个数的差异

/**
 * Helper methods to set/get next page id
 */
INDEX_TEMPLATE_ARGUMENTS
auto B_PLUS_TREE_LEAF_PAGE_TYPE::GetNextPageId() const -> page_id_t { return next_page_id_; }

INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_LEAF_PAGE_TYPE::SetNextPageId(page_id_t next_page_id) { next_page_id_ = next_page_id; }

/*
 * Helper method to find and return the key associated with input "index"(a.k.a
 * array offset)
 */
INDEX_TEMPLATE_ARGUMENTS
auto B_PLUS_TREE_LEAF_PAGE_TYPE::KeyAt(int index) const -> KeyType {
  // replace with your own code
  KeyType key{array_[index].first};
  return key;
}

// 下面自己加的

INDEX_TEMPLATE_ARGUMENTS
auto B_PLUS_TREE_LEAF_PAGE_TYPE::ValueAt(int index) const -> ValueType { return array_[index].second; }

// find和insert 都是可以二分优化的，后面再说

INDEX_TEMPLATE_ARGUMENTS
auto B_PLUS_TREE_LEAF_PAGE_TYPE::FindKey(const KeyType &key, const ValueType &value, KeyComparator comparator) -> int {
  for (int i = 0; i < GetSize(); i++) {
    if (comparator(array_[i].first, key) == 0) {
      return i;
    }
  }
  return GetSize();  // 这里没有判断不存在的情况，用类似end()来表示不存在
}

INDEX_TEMPLATE_ARGUMENTS
auto B_PLUS_TREE_LEAF_PAGE_TYPE::KeyIndex(const KeyType &key, const KeyComparator &keyComparator) const -> int {
  auto target = std::lower_bound(array_, array_ + GetSize(), key, [&keyComparator](const auto &pair, auto k) {
    return keyComparator(pair.first, k) < 0;
  });
  return std::distance(array_, target);
}

INDEX_TEMPLATE_ARGUMENTS
auto B_PLUS_TREE_LEAF_PAGE_TYPE::InsertKey(const KeyType &key, const ValueType &value, KeyComparator keyComparator)
    -> bool {
  // int pos = FindKey(key, value, comparator);
  // if (pos != GetSize()) {
  //   return false;  // 已经存在，则插入失败
  // }

  // // 插入一个

  // for (int i = GetSize() - 1; i >= 0; i--) {
  //   if (comparator(array_[i].first, key) == 1) {  // 1表示前面大于后面
  //     array_[i + 1] = array_[i];
  //   } else {
  //     array_[i + 1] = std::make_pair(key, value);
  //     break;
  //   }
  // }

  // IncreaseSize(1);

  // // 这里array_用了静态数组定义的，想不到什么好的方法，只有直接让他越界了。
  // // 因为定义的时候只定义了1个大小，相当于扩容的时候逻辑上扩容，但这种行为其实很危险，已经越界了
  // // 但是在内存上，至少是安全的，array_ 这个数组名就相当于指示kv链的开始位置，
  // // 并没有起到数组的作用，(说的有点乱，能理解就好，毕竟用vector的话，他自带的扩容机制，反而不安全)
  // // 这样直接操作内存分布反而是可控的。

  // return true;

  auto distance_in_array = KeyIndex(key, keyComparator);
  if (distance_in_array == GetSize()) {
    *(array_ + distance_in_array) = {key, value};
    IncreaseSize(1);
    return true;
  }

  if (keyComparator(array_[distance_in_array].first, key) == 0) {
    return true;
  }

  std::move_backward(array_ + distance_in_array, array_ + GetSize(), array_ + GetSize() + 1);
  *(array_ + distance_in_array) = {key, value};

  IncreaseSize(1);
  return true;
}

// 删除没什么二分优化
INDEX_TEMPLATE_ARGUMENTS
auto B_PLUS_TREE_LEAF_PAGE_TYPE::DeleteKey(const KeyType &key, const ValueType &value, KeyComparator comparator)
    -> bool {
  int pos = FindKey(key, value, comparator);
  if (pos == GetSize()) {
    return false;  // 不存在，则删除失败
  }

  for (int i = pos + 1; i < GetSize(); i--) {
    array_[i - 1] = array_[i];
  }

  IncreaseSize(-1);
  return true;
}

template class BPlusTreeLeafPage<GenericKey<4>, RID, GenericComparator<4>>;
template class BPlusTreeLeafPage<GenericKey<8>, RID, GenericComparator<8>>;
template class BPlusTreeLeafPage<GenericKey<16>, RID, GenericComparator<16>>;
template class BPlusTreeLeafPage<GenericKey<32>, RID, GenericComparator<32>>;
template class BPlusTreeLeafPage<GenericKey<64>, RID, GenericComparator<64>>;
}  // namespace bustub
