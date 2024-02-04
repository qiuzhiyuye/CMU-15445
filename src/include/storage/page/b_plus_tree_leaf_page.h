//===----------------------------------------------------------------------===//
//
//                         CMU-DB Project (15-445/645)
//                         ***DO NO SHARE PUBLICLY***
//
// Identification: src/include/page/b_plus_tree_leaf_page.h
//
// Copyright (c) 2018, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//
#pragma once

#include <utility>
#include <vector>

#include "storage/page/b_plus_tree_page.h"

namespace bustub {

#define B_PLUS_TREE_LEAF_PAGE_TYPE BPlusTreeLeafPage<KeyType, ValueType, KeyComparator>
#define LEAF_PAGE_HEADER_SIZE 28
// 儿子头的大小原先24+4
#define LEAF_PAGE_SIZE ((BUSTUB_PAGE_SIZE - LEAF_PAGE_HEADER_SIZE) / sizeof(MappingType))
// 这三个define和内部结点那里分析一样
/**
 * Store indexed key and record id(record id = page id combined with slot id,
 * see include/common/rid.h for detailed implementation) together within leaf
 * page. Only support unique key.
 *
 * // 只支持不同key！！！！！！！！！！
 * // 这里键值对的value是rid，不再是内部节点的下一个的指针
 *
 *
 * Leaf page format (keys are stored in order):
 *  ----------------------------------------------------------------------
 * | HEADER | KEY(1) + RID(1) | KEY(2) + RID(2) | ... | KEY(n) + RID(n)
 *  ----------------------------------------------------------------------
 *
 *  Header format (size in byte, 28 bytes in total):
 *  ---------------------------------------------------------------------
 * | PageType (4) | LSN (4) | CurrentSize (4) | MaxSize (4) |
 *  ---------------------------------------------------------------------
 *  -----------------------------------------------
 * | ParentPageId (4) | PageId (4) | NextPageId (4)
 *  -----------------------------------------------
 */
// 儿子结点的头就是在原先头的基础上 加上链表指针即可

INDEX_TEMPLATE_ARGUMENTS
class BPlusTreeLeafPage : public BPlusTreePage {
 public:
  // After creating a new leaf page from buffer pool, must call initialize
  // method to set default values
  void Init(page_id_t page_id, page_id_t parent_id = INVALID_PAGE_ID, int max_size = LEAF_PAGE_SIZE);
  // helper methods
  auto GetNextPageId() const -> page_id_t;
  void SetNextPageId(page_id_t next_page_id);
  auto KeyAt(int index) const -> KeyType;

  // 自己加的
  auto ValueAt(int index) const -> ValueType;

  auto InsertKey(const KeyType &key, const ValueType &value, KeyComparator keyComparator) -> bool;

  auto FindKey(const KeyType &key, const ValueType &value, KeyComparator comparator) -> int;

  auto DeleteKey(const KeyType &key, const ValueType &value, KeyComparator comparator) -> bool;

  auto KeyIndex(const KeyType &key, const KeyComparator &keyComparator) const -> int; 

  //  private:
  page_id_t next_page_id_;
  // Flexible array member for page data.
  MappingType array_[1];
};
}  // namespace bustub
