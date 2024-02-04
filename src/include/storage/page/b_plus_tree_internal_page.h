//===----------------------------------------------------------------------===//
//
//                         CMU-DB Project (15-445/645)
//                         ***DO NO SHARE PUBLICLY***
//
// Identification: src/include/page/b_plus_tree_internal_page.h
//
// Copyright (c) 2018, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//
#pragma once

#include <queue>

#include "storage/page/b_plus_tree_page.h"

namespace bustub {

#define B_PLUS_TREE_INTERNAL_PAGE_TYPE BPlusTreeInternalPage<KeyType, ValueType, KeyComparator>
// 这里面宏定义了一下内部结点的类型，可能为了方便看吧

#define INTERNAL_PAGE_HEADER_SIZE 24
// 头结点大小24B ，单位应该是B，因为下面相减了，而这里和磁盘交互的页面大小一般设为4KB，BUSTUB_PAGE_SIZE=4096
// 所以这单位应该是B
// 这里24 应该就是b_plus_tree_page 里面公有的24B头部信息。(那这里为什么24不用sizeof(b_plus_tree_page))
// 这样修改公有信息结构不是这也也不用修改了吗，不懂，我觉得我的方法好一点
// 看下面结点结构，就是24B+ 不知道多少对KV
#define INTERNAL_PAGE_SIZE ((BUSTUB_PAGE_SIZE - INTERNAL_PAGE_HEADER_SIZE) / (sizeof(MappingType)))
// 应该是一个页面当做一个结点，去掉结点头部信息，剩下的就是全部用来存放 键值对，MappingType，是pair，类似于键值对吧
// 这里就是计算一个页面(应该是把一个页面看成一个结点了，不然别的方案的话会很麻烦，听课的时候，说的也是数据库一般
// 一个页面看成一个结点)里面
// 能放多少对有效信息。
/**
 * Store n indexed keys and n+1 child pointers (page_id) within internal page.
 * // 为什么是 n个关键字 n+1个子树？？？ 408学的是n个子树
 * Pointer PAGE_ID(i) points to a subtree in which all keys K satisfy:
 * K(i) <= K < K(i+1).    //
 * 这里说的是按照K递增关系排序(模板定义的时候第三个是key比较方法，所以这不是普通的递增，需要满足传入得规定) NOTE: since
 * the number of keys does not equal to number of child pointers, the first key always remains invalid. That is to say,
 * any search/lookup should ignore the first key.
 * // 这个是讲怎么处理关键字和子树个数不一样的问题，忽略第一个的key
 *
 * Internal page format (keys are stored in increasing order):
 *  --------------------------------------------------------------------------
 * | HEADER | KEY(1)+PAGE_ID(1) | KEY(2)+PAGE_ID(2) | ... | KEY(n)+PAGE_ID(n) |
 *  --------------------------------------------------------------------------
 */

// 这也是模板，只不过被宏定义了一下，模板声明点进去看就行了
INDEX_TEMPLATE_ARGUMENTS
class BPlusTreeInternalPage : public BPlusTreePage {
 public:
  // must call initialize method after "create" a new node
  void Init(page_id_t page_id, page_id_t parent_id = INVALID_PAGE_ID, int max_size = INTERNAL_PAGE_SIZE);

  auto KeyAt(int index) const -> KeyType;
  void SetKeyAt(int index, const KeyType &key);
  auto ValueAt(int index) const -> ValueType;

  // 自己加的
  auto Insertkv(const ValueType &old_value, const KeyType &new_key, const ValueType &new_value) -> bool;
  void SetValueAt(int index, const ValueType &value);
  void SetAllAt(int index, const KeyType &key, const ValueType &value);
  //  private:
  // Flexible array member for page data.
  MappingType array_[1];
  // 约等于 pair<?,?> a[1];但为什么是1呢？ 后面找到答案了
};
}  // namespace bustub
