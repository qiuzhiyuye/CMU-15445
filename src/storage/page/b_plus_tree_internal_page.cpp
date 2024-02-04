//===----------------------------------------------------------------------===//
//
//                         CMU-DB Project (15-445/645)
//                         ***DO NO SHARE PUBLICLY***
//
// Identification: src/page/b_plus_tree_internal_page.cpp
//
// Copyright (c) 2018, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include <iostream>
#include <sstream>

#include "common/exception.h"
#include "storage/page/b_plus_tree_internal_page.h"

namespace bustub {
/*****************************************************************************
 * HELPER METHODS AND UTILITIES
 *****************************************************************************/
/*
 * Init method after creating a new internal page
 * Including set page type, set current size, set page id, set parent id and set
 * max page size
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::Init(page_id_t page_id, page_id_t parent_id, int max_size) {
  SetPageType(IndexPageType::INTERNAL_PAGE);
  SetSize(1);  // 这个size先默认为结点关键字个数
  SetPageId(page_id);
  SetParentPageId(parent_id);
  SetMaxSize(max_size);  // max_size 应该也是最大关键词个数，

  // 没要求LSN,这里先写上吧，这样6个头部信息初始化结束
  SetLSN();
}
// 为什么不是用构造函数？？？，要写一个init函数

// 为什么初始化size 为1 :
// 因为这里设计和408的子树和关键字个数是不同的，
// 然后忽略第一个点的key值，所以这个结点的array_[] 也就是下一级的儿子指针先初始化为了1，这样就可以忽视
// 第一个key了，所以上面setsize要为1，而不是0
// 其实也可能是我这里抄的这个代码这里不对，先待定再说吧

/*
 * Helper method to get/set the key associated with input "index"(a.k.a
 * array offset)
 */
INDEX_TEMPLATE_ARGUMENTS
auto B_PLUS_TREE_INTERNAL_PAGE_TYPE::KeyAt(int index) const -> KeyType {
  // replace with your own code
  // 也可以和下面统一起来
  // return array_[index].first;
  KeyType key{array_[index].first};
  return key;
}

// 这几个at函数，应该就是在array_ 的index 也就是找这个结点内的指向儿子指针的index

INDEX_TEMPLATE_ARGUMENTS
auto B_PLUS_TREE_INTERNAL_PAGE_TYPE::Insertkv(const ValueType &old_value, const KeyType &new_key,
                                              const ValueType &new_value) -> bool {
  int new_value_idx = 0;
  for (int i = 1; i < GetSize(); i++) {
    if (array_[i].second == old_value) {
      new_value_idx = i + 1;
      break;
    }
  }
  for (int i = GetSize() - 1; i >= new_value_idx; i--) {
    array_[i + 1] = array_[i];
  }

  array_[new_value_idx].first = new_key;
  array_[new_value_idx].second = new_value;

  IncreaseSize(1);
  return true;
}

INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::SetValueAt(int index, const ValueType &value) { array_[index].second = value; }

INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::SetAllAt(int index, const KeyType &key, const ValueType &value) {
  array_[index].first = key;
  array_[index].second = value;
}

INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::SetKeyAt(int index, const KeyType &key) { array_[index].first = key; }

/*
 * Helper method to get the value associated with input "index"(a.k.a array
 * offset)
 */
INDEX_TEMPLATE_ARGUMENTS
auto B_PLUS_TREE_INTERNAL_PAGE_TYPE::ValueAt(int index) const -> ValueType { return array_[index].second; }

// valuetype for internalNode should be page id_t
template class BPlusTreeInternalPage<GenericKey<4>, page_id_t, GenericComparator<4>>;
template class BPlusTreeInternalPage<GenericKey<8>, page_id_t, GenericComparator<8>>;
template class BPlusTreeInternalPage<GenericKey<16>, page_id_t, GenericComparator<16>>;
template class BPlusTreeInternalPage<GenericKey<32>, page_id_t, GenericComparator<32>>;
template class BPlusTreeInternalPage<GenericKey<64>, page_id_t, GenericComparator<64>>;
}  // namespace bustub
