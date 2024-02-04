//===----------------------------------------------------------------------===//
//
//                         CMU-DB Project (15-445/645)
//                         ***DO NO SHARE PUBLICLY***
//
// Identification: src/page/b_plus_tree_page.cpp
//
// Copyright (c) 2018, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "storage/page/b_plus_tree_page.h"

namespace bustub {

/*
 * Helper methods to get/set page type
 * Page type enum class is defined in b_plus_tree_page.h
 */
auto BPlusTreePage::IsLeafPage() const -> bool {
  return page_type_ == IndexPageType::LEAF_PAGE;
  // 枚举enum 的用法！！！学习一下,需要判断类型的一种处理方法
}
auto BPlusTreePage::IsRootPage() const -> bool {
  // 和上一个一起，这个类里面第一个难点，判断类型，上面那个主要是学一下语法，how
  // 这个是逻辑问题+不太想得到用INVALID_PAGE_ID，毕竟这里看不到这个变量
  // 这个是在config.h页面的东西，已经在include里面嵌套了好几次才找得到
  // 需要对整个项目有个全局观才能想得起来
  // rootpage：和acm一般操作一样，就是根结点的父节点设置为-1，即为不存在
  // 但是看了别人的才知道这里设置不存在可以用已经宏定义的INVALID_PAGE_ID来视为不存在
  // 其实这里做到后面也能看到答案，在后面建树的时候看得到这个应该用INVALID_PAGE_ID

  return parent_page_id_ == INVALID_PAGE_ID;
}
void BPlusTreePage::SetPageType(IndexPageType page_type) { page_type_ = page_type; }

auto BPlusTreePage::GetPageType() -> IndexPageType { return page_type_; }

/*
 * Helper methods to get/set size (number of key/value pairs stored in that
 * page)
 */
auto BPlusTreePage::GetSize() const -> int { return size_; }
void BPlusTreePage::SetSize(int size) { size_ = size; }
void BPlusTreePage::IncreaseSize(int amount) { size_ += amount; }

/*
 * Helper methods to get/set max size (capacity) of the page
 */
auto BPlusTreePage::GetMaxSize() const -> int { return max_size_; }
void BPlusTreePage::SetMaxSize(int size) { max_size_ = size; }
// 这里的size应该是关键字个数

/*
 * Helper method to get min page size
 * Generally, min page size == max page size / 2
 */
auto BPlusTreePage::GetMinSize() const -> int { return (max_size_ + 1) / 2; }
// 这里注释没给清楚，最小个数应该是上取整，还好408学过，
// 也好理解的，不然得话max_size =1的话，min就是0了
// 这个类里面第二个难点，其他都是简单的get/set

/*
 * Helper methods to get/set parent page id
 */
auto BPlusTreePage::GetParentPageId() const -> page_id_t { return parent_page_id_; }
void BPlusTreePage::SetParentPageId(page_id_t parent_page_id) { parent_page_id_ = parent_page_id; }

/*
 * Helper methods to get/set self page id
 */
auto BPlusTreePage::GetPageId() const -> page_id_t { return page_id_; }
void BPlusTreePage::SetPageId(page_id_t page_id) { page_id_ = page_id; }

/*
 * Helper methods to set lsn
 */
void BPlusTreePage::SetLSN(lsn_t lsn) { lsn_ = lsn; }

}  // namespace bustub
