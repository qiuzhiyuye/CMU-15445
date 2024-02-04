#include <string>

#include "common/exception.h"
#include "common/logger.h"
#include "common/rid.h"
#include "storage/index/b_plus_tree.h"
#include "storage/page/header_page.h"

namespace bustub {
INDEX_TEMPLATE_ARGUMENTS
BPLUSTREE_TYPE::BPlusTree(std::string name, BufferPoolManager *buffer_pool_manager, const KeyComparator &comparator,
                          int leaf_max_size, int internal_max_size)
    : index_name_(std::move(name)),
      root_page_id_(INVALID_PAGE_ID),
      buffer_pool_manager_(buffer_pool_manager),
      comparator_(comparator),
      leaf_max_size_(leaf_max_size),
      internal_max_size_(internal_max_size) {}

// 6个数据全部初始了，应该没别的了吧

/*
 * Helper function to decide whether current b+tree is empty
 */
INDEX_TEMPLATE_ARGUMENTS
auto BPLUSTREE_TYPE::IsEmpty() const -> bool {
  return root_page_id_ == INVALID_PAGE_ID;
  // 初始化状态一定是空的，应该显然
  // 显然插入第一个点的时候，需要做特判处理，当然这是后面插入考虑的事情。
}
INDEX_TEMPLATE_ARGUMENTS
auto BPLUSTREE_TYPE::Findleaf(const KeyType &key) -> LeafPage * {
  // 遇到个很奇怪的bug，一样的内容，我原先format之后，auto和后面会自动分开一行
  // 但是我从新复制一遍从写一遍，再format就不会了

  // 这个函数不判断b+树空的，在外面自己判断
  // 和普通遍历树一样，先临时移动结点指向根节点，然后慢慢往叶子结点找
  // 1. auto *==>  auto在这里看样子是不能自动识别出来指针类型的，
  // 2.reinterpret_cast<A*>(a) 内存不变，只是内存上的内容的解释方式改变了
  // A是类型，a是内容
  auto *page = reinterpret_cast<BPlusTreePage *>(buffer_pool_manager_->FetchPage(root_page_id_)->GetData());
  // 这句话就是用缓存池取出root页面的内容，然后把他解释成为b+树页面的指针
  // 现在一个问题->getdata要不要有
  // 不加的话，返回值是page* ,加上的话返回值是char* 个人感觉都行，因为都是那块内容
  // 去掉后：
  if (page == nullptr) {
    LOG_INFO("FindLeaf 根结点不是B+页面");
  }
  while (!page->IsLeafPage()) {
    auto *internal = reinterpret_cast<InternalPage *>(page);
    if (page == nullptr) {
      LOG_INFO("FindLeaf 页面不是B+内部页面");
    }
    int idx = 1;
    //   link0 v1 link1   v2 link2  v3  link3
    for (idx = 1; idx < internal->GetSize(); idx++) {
      // 这里选择了遍历查找，实际上可以选择二分的,优化性能再考虑
      if (comparator_(internal->KeyAt(idx), key) == 1) {  // 找到一个比当前位置大的v时
        // 那么下一个遍历的点一定是当前idx-1的链接
        // 这样就算循环结束没有break，那么最终答案就是最后一个link，也是对的

        // 还要一个问题是v1，v2里面是什么值，这个应该是后面插入应该考虑的事情，
        // 这里这么写，主要是先看了别人的代码理解的，但实际上这里要和插入一起来考虑这里如何处理
        // 显然这里v1记录的是link1子树里面的最小值
        // 这个和408和一般市面上不一样，408是记录子树中的最大值
        // 这里只是实现细节不一样而已，不用太在意
        break;
      }
    }
    page = reinterpret_cast<BPlusTreePage *>(buffer_pool_manager_->FetchPage(internal->ValueAt(idx - 1))->GetData());
    buffer_pool_manager_->UnpinPage(internal->GetPageId(), false);
    // fetch的时候自动对页面pin了，也就是类似于上锁了，防止被调出内存
    // 这里页面没有任何变化，所以脏位改0即可。
    // 突然理解了这里unpin的脏位处理，突然意识到这里是唯一一个能处理脏位的地方，
    // 平时理解脏位都是上帝视角，但这里是唯一一个能维护脏位的方法。
    // 这里是解pin，页面用好了就得unpin，很容易忘记
  }
  return reinterpret_cast<LeafPage *>(page);
}

/*****************************************************************************
 * SEARCH
 *****************************************************************************/
/*
 * Return the only value that associated with input key
 * This method is used for point query
 * @return : true means key exists
 *
 * // compare : k1 < k2 -1
 *              k1 = k2 0
 *              k1 > k2 1
 *
 */
INDEX_TEMPLATE_ARGUMENTS
auto BPLUSTREE_TYPE::GetValue(const KeyType &key, std::vector<ValueType> *result, Transaction *transaction) -> bool {
  if (IsEmpty()) {
    return false;
  }
  // 找到叶子结点
  auto *leaf = Findleaf(key);
  // auto 判断不了指针，指针的*要自己加

  if (leaf == nullptr) {
    LOG_INFO("getvalue 叶子页面为空");
    return false;
    // 这里一定要加return 检查格式的时候，要求，对一个指针的空情况做讨论
    // 这里不加return 的话，就相当于没检查这个nullptr状态
    // 查了好久的错，以为别的地方错了
    // 纯粹格式要求
  }
  // 叶子里面对应页面从0开始，和内部结点不一样，别忘了

  // leaf->getsize这是父类的方法，没有重写的话也是可以直接用的，不需要别的操作
  // 找上面那个return bug的时候以为这里出错了，调用父类方法需要什么类型转换之类的，其实不需要
  for (int idx = 0; idx < leaf->GetSize(); idx++) {
    if (comparator_(leaf->KeyAt(idx), key)) {
      result->push_back(leaf->ValueAt(idx));
      // break;
      // 因为这里是是用vector来接的，所以我这先不break了，到时候过了再测试要不要加
    }
  }
  // 因为这个是key unique的树，所以最多只有一个答案？
  // 但是为什么给定的函数要这里用vector来传答案，不是很清楚，是因为方便以后改成不是unique的吗
  buffer_pool_manager_->UnpinPage(leaf->GetPageId(), false);
  // findleaf里面fetch了，这里别忘记unpin，因为是只读，所以脏位一直i是false；

  return !result->empty();
}

/*****************************************************************************
 * INSERTION
 *****************************************************************************/
/*
 * Insert constant key & value pair into b+ tree
 * if current tree is empty, start new tree, update root page id and insert
 * entry, otherwise insert into leaf page.
 * @return: since we only support unique key, if user try to insert duplicate
 * keys return false, otherwise return true.
 */
INDEX_TEMPLATE_ARGUMENTS
template <typename T>
auto BPLUSTREE_TYPE::Split(T *node) -> T * {
  // 最后返回值是新页面的page指针
  // 先新建一个相同类型页，因为分裂结点是内部或者叶子都有可能，所以用模板
  page_id_t new_page_id;
  auto *new_page = reinterpret_cast<T *>(buffer_pool_manager_->NewPage(&new_page_id)->GetData());
  new_page->SetPageType(node->GetPageType());

  // 为什么要分类初始化信息，因为，他们的maxsize是不同的，要分类init
  if (node->IsLeafPage()) {  // 是叶子结点
    auto *old_leaf = reinterpret_cast<LeafPage *>(node);
    auto *new_leaf = reinterpret_cast<LeafPage *>(new_page);

    new_leaf->Init(new_page_id, old_leaf->GetParentPageId(), leaf_max_size_);

    // 下面是叶子结点移动一半的代码
    int st_index = old_leaf->GetMinSize();  // st_index设为x
    // 所以old里面是0~x-1  new里面是x~结束
    std::copy(old_leaf->array_ + st_index, old_leaf->array_ + old_leaf->GetSize(),
              new_leaf->array_ + new_leaf->GetSize());
    // copy函数参数是    开始，结束位置的下一个，类似于end，放入的位置
    new_leaf->IncreaseSize(old_leaf->GetMaxSize() - st_index);
    old_leaf->SetSize(st_index);

  } else {  // 是内部结点，不同于叶子结点内部结点移动的时候还要把对应儿子的父节点改变，所以复杂很多很多！！！
    // 因为叶子结点所指向的页是父点的记录的。
    auto *old_internal = reinterpret_cast<InternalPage *>(node);
    auto *new_internal = reinterpret_cast<InternalPage *>(new_page);

    new_internal->Init(new_page_id, old_internal->GetParentPageId(), internal_max_size_);

    int st_index = old_internal->GetMinSize();
    for (int i = 0; i < st_index; i++) {
      auto *son_page =
          reinterpret_cast<BPlusTreePage *>(buffer_pool_manager_->FetchPage(old_internal->ValueAt(i + st_index)));
      son_page->SetParentPageId(new_page_id);
      buffer_pool_manager_->UnpinPage(son_page->GetPageId(), true);
    }
    std::copy(old_internal->array_ + st_index, old_internal->array_ + old_internal->GetSize(),
              new_internal->array_ + new_internal->GetSize());
    new_internal->IncreaseSize(old_internal->GetMaxSize() - st_index);
    old_internal->SetSize(st_index);
  }
  return new_page;  // 最后返回传入页面的类型的指针即可
}
INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::InsertIntoParent(BPlusTreePage *old_node, const KeyType &key, BPlusTreePage *new_node,
                                      Transaction *transaction) {
  // 参数表里面key传不传都行，函数里面拿也是可以的
  // key就是new_node里面的最小值

  //  如果原先的点是根结点，那么需要特判，因为需要新建一个父结点
  // 否则父节点是直接拿到的,而且父结点在外面已经设置好了
  // 因为split的时候父节点是直接抄的旧点的父节点，所以已经设置过了，这里两种情况要分清楚
  // 虽然不分也没什么关系，多设置一遍就多设置一遍
  if (old_node->IsRootPage()) {
    auto *new_root = reinterpret_cast<InternalPage *>(buffer_pool_manager_->NewPage(&root_page_id_)->GetData());
    // 初始化根结点信息
    new_root->Init(root_page_id_, INVALID_PAGE_ID, internal_max_size_);

    old_node->SetParentPageId(root_page_id_);
    new_node->SetParentPageId(root_page_id_);

    // 构建新根的array_数组
    new_root->SetAllAt(1, key, new_node->GetPageId());
    new_root->SetValueAt(0, old_node->GetPageId());
    // new_root->array_[0].second = old_node->GetPageId();
    // new_root->array_[1].first = key;
    // new_root->array_[1].second = new_node->GetPageId();
    new_root->SetSize(2);  // 设置大小容易忘记！！！

    UpdateRootPageId(0);  // debug函数，当根变化时候，调用，参数为0表示根是更新，而不是新建

    // unpin新的根
    buffer_pool_manager_->UnpinPage(new_root->GetPageId(), true);

    // 一个抉择点：两个点的unpin时机是什么：
    // 我这里选择了在函数内部就unpin，这样意味着这个函数代表了
    // 两个点关系插入父节点之后，同时unpin了
    buffer_pool_manager_->UnpinPage(old_node->GetPageId(), true);
    buffer_pool_manager_->UnpinPage(new_node->GetPageId(), true);
    return;
  }
  auto parent_page = buffer_pool_manager_->FetchPage(old_node->GetParentPageId());
  auto parent_node = reinterpret_cast<InternalPage *>(parent_page->GetData());
  // 直接拿到父节点

  if (parent_node->GetSize() < internal_max_size_) {  // 父节点没有满，直接插入
    // 儿子结点对父亲关系这个函数外面新建点的时候已经搞定了，
    // 现在只需要把关系插入父点的array_数组里面即可
    parent_node->Insertkv(old_node->GetPageId(), key, new_node->GetPageId());
    // unpin 父节点
    buffer_pool_manager_->UnpinPage(parent_node->GetPageId(), true);

    buffer_pool_manager_->UnpinPage(old_node->GetPageId(), true);
    buffer_pool_manager_->UnpinPage(new_node->GetPageId(), true);
    return;
  }
  // 父节点满了，需要递归向上插入
  auto *mem = new char[INTERNAL_PAGE_HEADER_SIZE + sizeof(MappingType) * (parent_node->GetSize() + 1)];
  auto *copy_parent_node = reinterpret_cast<InternalPage *>(mem);
  // 直接new出新空间结点，然后memcpy过去
  // 因为父节点满了，我们需要插入之后才能分裂，所以，需要对他做一个备份，多一个map位置
  // 的空间，再去insertkv进去，然后在进行分裂，再把分裂后属于原来的结点copy回去，
  // 一定要覆盖掉后面的内容，不是只对前一半进行赋值
  std::memcpy(mem, parent_page->GetData(), INTERNAL_PAGE_HEADER_SIZE + sizeof(MappingType) * (parent_node->GetSize()));
  copy_parent_node->Insertkv(old_node->GetPageId(), key, new_node->GetPageId());
  auto parent_new_sibiling_node = Split(copy_parent_node);
  // 设置原本节点的大小这些事在split里面已经实现了，外面不需要处理了

  KeyType new_key = parent_new_sibiling_node->KeyAt(0);
  std::memcpy(parent_page->GetData(), mem,
              INTERNAL_PAGE_HEADER_SIZE + sizeof(MappingType) * copy_parent_node->GetMinSize());
  // memcpy的参数是 赋值后的空间起始位置，赋值内容的起始位置，赋值长度
  InsertIntoParent(parent_node, new_key, parent_new_sibiling_node, transaction);
  // 因为我选择了InsertIntoParent内部就unpin结束两个点，所以，这里递归之后不用unpin了
  // 但是上面两个分支是递归结束部分，必须要unpin

  // 向上递归插入父节点和父节点的新兄弟
  delete[] mem;  // new之后要delete，而且这里是数组删除
}

INDEX_TEMPLATE_ARGUMENTS
auto BPLUSTREE_TYPE::Insert(const KeyType &key, const ValueType &value, Transaction *transaction) -> bool {
  std::vector<ValueType> result;
  if (GetValue(key, &result, transaction)) {  // 这个树里面已经有这个值了，就直接返回false
    return false;
  }  // 这里可以做一部分优化，毕竟可以边查边判断有没有，不然大概率这个getvalue函数是白跑一趟的

  // 空树，第一次插入
  // root结点同时也是一个叶子结点
  if (IsEmpty()) {
    auto *leaf = reinterpret_cast<LeafPage *>(buffer_pool_manager_->NewPage(&root_page_id_)->GetData());
    // 这里新建一个页面，同时，把页面号直接传给root_page_id
    leaf->Init(root_page_id_, INVALID_PAGE_ID, leaf_max_size_);
    // 这个页面初始化 页面号，父节点页面号，最大size
    leaf->InsertKey(key, value, comparator_);
    // 把kv插入这个结点中,插入函数是在叶子结点类里面自行实现的
    // 反正插入只会在叶子结点，所以不用担心

    UpdateRootPageId(1);  // 这个是调试函数，root更新的时候调用，到时候用来debug

    buffer_pool_manager_->UnpinPage(root_page_id_, true);
    // 内容更改了，头部信息改了(头部信息就是这个页面内容中的内容)，所以脏位是1
    return true;
  }

  auto *leaf = Findleaf(key);
  if (leaf == nullptr) {
    return false;
  }

  leaf->InsertKey(key, value, comparator_);
  // 这里选择先插入，满了立马分裂，而不是插入前面判断满不满
  // 其实这里的分裂时机两种都行，可以自由选择。
  // 感觉先插入再判断满好写点，不然插入那个点位置不好放
  // 需要两个页面判断插在哪个页面里面

  // 插入的时候不需要考虑最小size的情况，因为，当根节点就是叶子结点的时候
  // size会小于最小size，这种情况插入结束即可，不用管，只有删除才要考虑这种情况
  if (leaf->GetSize() == leaf->GetMaxSize()) {  // 页面已经满了，需要分裂
    auto sibling_leaf_node = Split(leaf);
    // 维护一下叶子链的关系
    sibling_leaf_node->SetNextPageId(leaf->GetNextPageId());
    leaf->SetNextPageId(sibling_leaf_node->GetPageId());

    // 分裂之后就是递归，向上插入一个点
    // 参数是左点，右点最小值，右点和tansaction，
    // tansaction是后面p用的，先不用管
    InsertIntoParent(leaf, sibling_leaf_node->KeyAt(0), sibling_leaf_node, transaction);

    // 两个页面用完了，unpin
    // 这里换了一个实现，unpin直接放在InsertIntoParent函数里面了，
    // 这样减少这里的unpin和里面递归时候的unpin
    // buffer_pool_manager_->UnpinPage(leaf->GetPageId(),true);
    // buffer_pool_manager_->UnpinPage(sibling_leaf_node->GetPageId(),true);
  } else {  // 没有满
    buffer_pool_manager_->UnpinPage(leaf->GetPageId(), true);
    // 这个页面多了一个kv对，所以应该是修改了，所以脏位是1
  }



  return true;
}

/*****************************************************************************
 * REMOVE
 *****************************************************************************/
/*
 * Delete key & value pair associated with input key
 * If current tree is empty, return immdiately.
 * If not, User needs to first find the right leaf page as deletion target, then
 * delete entry from leaf page. Remember to deal with redistribute or merge if
 * necessary.
 */
INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::Remove(const KeyType &key, Transaction *transaction) {}

/*****************************************************************************
 * INDEX ITERATOR
 *****************************************************************************/
/*
 * Input parameter is void, find the leaftmost leaf page first, then construct
 * index iterator
 * @return : index iterator
 */
INDEX_TEMPLATE_ARGUMENTS
auto BPLUSTREE_TYPE::Begin() -> INDEXITERATOR_TYPE { return INDEXITERATOR_TYPE(); }

/*
 * Input parameter is low key, find the leaf page that contains the input key
 * first, then construct index iterator
 * @return : index iterator
 */
INDEX_TEMPLATE_ARGUMENTS
auto BPLUSTREE_TYPE::Begin(const KeyType &key) -> INDEXITERATOR_TYPE { return INDEXITERATOR_TYPE(); }

/*
 * Input parameter is void, construct an index iterator representing the end
 * of the key/value pair in the leaf node
 * @return : index iterator
 */
INDEX_TEMPLATE_ARGUMENTS
auto BPLUSTREE_TYPE::End() -> INDEXITERATOR_TYPE { return INDEXITERATOR_TYPE(); }

/**
 * @return Page id of the root of this tree
 */
INDEX_TEMPLATE_ARGUMENTS
auto BPLUSTREE_TYPE::GetRootPageId() -> page_id_t { return root_page_id_; }

/*****************************************************************************
 * UTILITIES AND DEBUG
 *****************************************************************************/
// 下面应该是给定的调试工具了，不需要我们管
// 上面才是我们需要实现B+ 的函数

/*
 * Update/Insert root page id in header page(where page_id = 0, header_page is
 * defined under include/page/header_page.h)
 * Call this method everytime root page id is changed.
 * @parameter: insert_record      defualt value is false. When set to true,
 * insert a record <index_name, root_page_id> into header page instead of
 * updating it.
 */
INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::UpdateRootPageId(int insert_record) {
  auto *header_page = static_cast<HeaderPage *>(buffer_pool_manager_->FetchPage(HEADER_PAGE_ID));
  if (insert_record != 0) {
    // create a new record<index_name + root_page_id> in header_page
    header_page->InsertRecord(index_name_, root_page_id_);
  } else {
    // update root_page_id in header_page
    header_page->UpdateRecord(index_name_, root_page_id_);
  }
  buffer_pool_manager_->UnpinPage(HEADER_PAGE_ID, true);
}

/*
 * This method is used for test only
 * Read data from file and insert one by one
 */
INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::InsertFromFile(const std::string &file_name, Transaction *transaction) {
  int64_t key;
  std::ifstream input(file_name);
  while (input) {
    input >> key;

    KeyType index_key;
    index_key.SetFromInteger(key);
    RID rid(key);
    Insert(index_key, rid, transaction);
  }
}
/*
 * This method is used for test only
 * Read data from file and remove one by one
 */
INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::RemoveFromFile(const std::string &file_name, Transaction *transaction) {
  int64_t key;
  std::ifstream input(file_name);
  while (input) {
    input >> key;
    KeyType index_key;
    index_key.SetFromInteger(key);
    Remove(index_key, transaction);
  }
}

/**
 * This method is used for debug only, You don't need to modify
 */
INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::Draw(BufferPoolManager *bpm, const std::string &outf) {
  if (IsEmpty()) {
    LOG_WARN("Draw an empty tree");
    return;
  }
  std::ofstream out(outf);
  out << "digraph G {" << std::endl;
  ToGraph(reinterpret_cast<BPlusTreePage *>(bpm->FetchPage(root_page_id_)->GetData()), bpm, out);
  out << "}" << std::endl;
  out.flush();
  out.close();
}

/**
 * This method is used for debug only, You don't need to modify
 */
INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::Print(BufferPoolManager *bpm) {
  if (IsEmpty()) {
    LOG_WARN("Print an empty tree");
    return;
  }
  ToString(reinterpret_cast<BPlusTreePage *>(bpm->FetchPage(root_page_id_)->GetData()), bpm);
}

/**
 * This method is used for debug only, You don't need to modify
 * @tparam KeyType
 * @tparam ValueType
 * @tparam KeyComparator
 * @param page
 * @param bpm
 * @param out
 */
INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::ToGraph(BPlusTreePage *page, BufferPoolManager *bpm, std::ofstream &out) const {
  std::string leaf_prefix("LEAF_");
  std::string internal_prefix("INT_");
  if (page->IsLeafPage()) {
    auto *leaf = reinterpret_cast<LeafPage *>(page);
    // Print node name
    out << leaf_prefix << leaf->GetPageId();
    // Print node properties
    out << "[shape=plain color=green ";
    // Print data of the node
    out << "label=<<TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\" CELLPADDING=\"4\">\n";
    // Print data
    out << "<TR><TD COLSPAN=\"" << leaf->GetSize() << "\">P=" << leaf->GetPageId() << "</TD></TR>\n";
    out << "<TR><TD COLSPAN=\"" << leaf->GetSize() << "\">"
        << "max_size=" << leaf->GetMaxSize() << ",min_size=" << leaf->GetMinSize() << ",size=" << leaf->GetSize()
        << "</TD></TR>\n";
    out << "<TR>";
    for (int i = 0; i < leaf->GetSize(); i++) {
      out << "<TD>" << leaf->KeyAt(i) << "</TD>\n";
    }
    out << "</TR>";
    // Print table end
    out << "</TABLE>>];\n";
    // Print Leaf node link if there is a next page
    if (leaf->GetNextPageId() != INVALID_PAGE_ID) {
      out << leaf_prefix << leaf->GetPageId() << " -> " << leaf_prefix << leaf->GetNextPageId() << ";\n";
      out << "{rank=same " << leaf_prefix << leaf->GetPageId() << " " << leaf_prefix << leaf->GetNextPageId() << "};\n";
    }

    // Print parent links if there is a parent
    if (leaf->GetParentPageId() != INVALID_PAGE_ID) {
      out << internal_prefix << leaf->GetParentPageId() << ":p" << leaf->GetPageId() << " -> " << leaf_prefix
          << leaf->GetPageId() << ";\n";
    }
  } else {
    auto *inner = reinterpret_cast<InternalPage *>(page);
    // Print node name
    out << internal_prefix << inner->GetPageId();
    // Print node properties
    out << "[shape=plain color=pink ";  // why not?
    // Print data of the node
    out << "label=<<TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\" CELLPADDING=\"4\">\n";
    // Print data
    out << "<TR><TD COLSPAN=\"" << inner->GetSize() << "\">P=" << inner->GetPageId() << "</TD></TR>\n";
    out << "<TR><TD COLSPAN=\"" << inner->GetSize() << "\">"
        << "max_size=" << inner->GetMaxSize() << ",min_size=" << inner->GetMinSize() << ",size=" << inner->GetSize()
        << "</TD></TR>\n";
    out << "<TR>";
    for (int i = 0; i < inner->GetSize(); i++) {
      out << "<TD PORT=\"p" << inner->ValueAt(i) << "\">";
      if (i > 0) {
        out << inner->KeyAt(i);
      } else {
        out << " ";
      }
      out << "</TD>\n";
    }
    out << "</TR>";
    // Print table end
    out << "</TABLE>>];\n";
    // Print Parent link
    if (inner->GetParentPageId() != INVALID_PAGE_ID) {
      out << internal_prefix << inner->GetParentPageId() << ":p" << inner->GetPageId() << " -> " << internal_prefix
          << inner->GetPageId() << ";\n";
    }
    // Print leaves
    for (int i = 0; i < inner->GetSize(); i++) {
      auto child_page = reinterpret_cast<BPlusTreePage *>(bpm->FetchPage(inner->ValueAt(i))->GetData());
      ToGraph(child_page, bpm, out);
      if (i > 0) {
        auto sibling_page = reinterpret_cast<BPlusTreePage *>(bpm->FetchPage(inner->ValueAt(i - 1))->GetData());
        if (!sibling_page->IsLeafPage() && !child_page->IsLeafPage()) {
          out << "{rank=same " << internal_prefix << sibling_page->GetPageId() << " " << internal_prefix
              << child_page->GetPageId() << "};\n";
        }
        bpm->UnpinPage(sibling_page->GetPageId(), false);
      }
    }
  }
  bpm->UnpinPage(page->GetPageId(), false);
}

/**
 * This function is for debug only, you don't need to modify
 * @tparam KeyType
 * @tparam ValueType
 * @tparam KeyComparator
 * @param page
 * @param bpm
 */
INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::ToString(BPlusTreePage *page, BufferPoolManager *bpm) const {
  if (page->IsLeafPage()) {
    auto *leaf = reinterpret_cast<LeafPage *>(page);
    std::cout << "Leaf Page: " << leaf->GetPageId() << " parent: " << leaf->GetParentPageId()
              << " next: " << leaf->GetNextPageId() << std::endl;
    for (int i = 0; i < leaf->GetSize(); i++) {
      std::cout << leaf->KeyAt(i) << ",";
    }
    std::cout << std::endl;
    std::cout << std::endl;
  } else {
    auto *internal = reinterpret_cast<InternalPage *>(page);
    std::cout << "Internal Page: " << internal->GetPageId() << " parent: " << internal->GetParentPageId() << std::endl;
    for (int i = 0; i < internal->GetSize(); i++) {
      std::cout << internal->KeyAt(i) << ": " << internal->ValueAt(i) << ",";
    }
    std::cout << std::endl;
    std::cout << std::endl;
    for (int i = 0; i < internal->GetSize(); i++) {
      ToString(reinterpret_cast<BPlusTreePage *>(bpm->FetchPage(internal->ValueAt(i))->GetData()), bpm);
    }
  }
  bpm->UnpinPage(page->GetPageId(), false);
}

template class BPlusTree<GenericKey<4>, RID, GenericComparator<4>>;
template class BPlusTree<GenericKey<8>, RID, GenericComparator<8>>;
template class BPlusTree<GenericKey<16>, RID, GenericComparator<16>>;
template class BPlusTree<GenericKey<32>, RID, GenericComparator<32>>;
template class BPlusTree<GenericKey<64>, RID, GenericComparator<64>>;

}  // namespace bustub
