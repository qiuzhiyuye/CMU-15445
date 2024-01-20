//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// lru_k_replacer.cpp
//
// Identification: src/buffer/lru_k_replacer.cpp
//
// Copyright (c) 2015-2022, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "buffer/lru_k_replacer.h"
#include "common/logger.h"

namespace bustub {  // 函数定义全在.h文件，这里全是实现部分，完成所有函数应该就结束了

LRUKReplacer::LRUKReplacer(size_t num_frames, size_t k) : replacer_size_(num_frames), k_(k) {
  // 四个变量，两个已经给了赋值了，另外两个定义已经初始化为0了，这里应该空就行了，最多自己加变量才会动
  lru_head_ = std::make_shared<Frame>();
  lru_tail_ = std::make_shared<Frame>();
  history_head_ = std::make_shared<Frame>();
  history_tail_ = std::make_shared<Frame>();
  // 这里结构是make_shared<T>(T的构造函数)

  lru_head_->nex_ = lru_tail_;
  lru_tail_->pre_ = lru_head_;
  history_head_->nex_ = history_tail_;
  history_tail_->pre_ = history_head_;
  // LOG_INFO("%d %d",(int)L.use_count(),(int)R.use_count());
  // 测试引用个数，看看有没有循环引用而内存泄漏
  // 这里要用weak_ptr解决循环引用的问题

  // 以下是测试链表是否可行  包括对weak_ptr的使用
  // weak_ptr 不能直接获取对象，需要.lock()判断是否还存在，要是在的话自动转为shared_ptr
  // std::shared_ptr<Frame> addnode(new Frame(2));
  // std::shared_ptr<Frame> addnode2(new Frame(4));
  // InsertToListHead(addnode,lru_head_);
  // InsertToListHead(addnode2,lru_head_);
  // RemoveFromList(addnode2);
  // auto who=(lru_head_->nex_).lock();
  // LOG_INFO("%d",(int)(who->Frame_id_));

  // 智能指针.reset() 可以手动释放(减1引用)
}

auto LRUKReplacer::Evict(frame_id_t *frame_id) -> bool {  // 这里传参数是传回去，不是传入删除的frame编号
  std::lock_guard<std::mutex> lock(latch_);
  // LOG_INFO("Evict");
  if (Size() == 0) {
    return false;
  }  // 没有可以驱逐的frame，包括有些被锁上了不给驱逐

  // 先驱逐history队列
  auto p = history_tail_->pre_.lock();
  while (p != history_head_) {
    if (p->evictable_) {
      *frame_id = p->frame_id_;

      RemoveFromList(p);
      mp_.erase(p->frame_id_);
      tot_size_--;
      curr_size_--;  // 本来这四条可以用remove 函数合并掉的，但是，因为
      // 并发锁不好控制，所以直接抄过来得了
      return true;
    }
    p = p->pre_.lock();
  }
  p = lru_tail_->pre_.lock();
  while (p != lru_head_) {
    if (p->evictable_) {
      *frame_id = p->frame_id_;

      RemoveFromList(p);
      mp_.erase(p->frame_id_);
      tot_size_--;
      curr_size_--;
      return true;
    }
    p = p->pre_.lock();
  }

  return false;
}

void LRUKReplacer::RecordAccess(frame_id_t frame_id) {
  std::lock_guard<std::mutex> lock(latch_);
  //  LOG_INFO("RecordAccess: %d", frame_id);
  if (frame_id > static_cast<frame_id_t>(replacer_size_)) {
    throw("插入了大于总空间的frame块了");
  }

  ++current_timestamp_;
  if (mp_.find(frame_id) == mp_.end()) {
    mp_[frame_id] = std::make_shared<Frame>(frame_id);
  }  // 没访问过，新建一个新结点
  // 下面维护时间戳
  auto p = mp_[frame_id];
  if (p->timestamps_.size() == k_) {
    p->timestamps_.pop_front();
  }
  p->record_cnt_++;
  p->timestamps_.push_back(current_timestamp_);
  // 下面维护队列
  if (p->record_cnt_ == 1) {
    if (tot_size_ == replacer_size_) {
      frame_id_t evict_id;
      while (!Evict(&evict_id)) {
        LOG_INFO("有新页插入，但是所有页面都不能驱逐");
      }  // 因为会出现全部锁住情况，所以这里死循环跑死得了！！！
         // 等待解锁页面
    }
    InsertToListHead(p, history_head_);
    curr_size_++;
    tot_size_++;
  }  // 新插入

  if (p->record_cnt_ >= k_) {
    RemoveFromList(p);
    InsertToListHead(p, lru_head_);
  }  // 要移动队列了

  // 在历史队列里面，也需要LRU处理
  // RemoveFromList(p);
  // InsertToListHead(p,history_head_);
  // 这里不是也需要LRU 处理吗，但是为什么加上反而是错的，很奇怪
  // 不加上就是FIFO处理
  // 哦，冲突了，文件里面注释要求是LRU，但是课程页面是写着要FIFO
  // 最后数据给的是FIFO
}

void LRUKReplacer::SetEvictable(frame_id_t frame_id, bool set_evictable) {
  std::lock_guard<std::mutex> lock(latch_);
  // LOG_INFO("SetEvictable: %d %d", frame_id,set_evictable);
  if (mp_.find(frame_id) == mp_.end()) {
    return;
  }  // 找不到
  // 设置对应结点即可，维护一下currsize参数即可
  auto p = mp_[frame_id];
  if (p->evictable_ == set_evictable) {
    return;
  }
  if (set_evictable) {
    curr_size_++;
  } else {
    curr_size_--;
  }
  p->evictable_ = set_evictable;
}

void LRUKReplacer::Remove(frame_id_t frame_id) {
  std::lock_guard<std::mutex> lock(latch_);
  // LOG_INFO("REMOVE: %d", frame_id);
  if (mp_.find(frame_id) == mp_.end()) {
    return;
  }  // 找不到的情况
  auto p = mp_[frame_id];
  if (!p->evictable_) {
    throw("删除了不可以驱逐的frame");
    return;
  }

  RemoveFromList(p);
  mp_.erase(frame_id);  // 链表里面删除，映射关系删除
  tot_size_--;
  curr_size_--;  // 大小关系维护一下
}

auto LRUKReplacer::Size() -> size_t {
  return curr_size_;
}  // 这里应该是返回去掉驱逐的之后的size,其实返回这个好像也没啥用，根本用不到
// 那个tot_size_ ，两个链表的长度和，大不了直接读哪个数据

// 自己加的
void LRUKReplacer::RemoveFromList(const std::shared_ptr<Frame> &frame_) {
  // 这里应该不需要区分在那个链表里面，只要移除了就行
  // st表示是否保留这个点，true表示删除这个点，不然这里不reset的话，这个点内存一直要到结束才能释放，虽然要是不管的话也没有什么问题

  std::shared_ptr<Frame> prev(frame_->pre_);
  std::shared_ptr<Frame> ne(frame_->nex_);
  prev->nex_ = ne;
  ne->pre_ = prev;
}
void LRUKReplacer::InsertToListHead(const std::shared_ptr<Frame> &frame_, const std::shared_ptr<Frame> &head) {
  std::shared_ptr<Frame> temp(head->nex_);
  // head addpoint temp
  head->nex_ = frame_;
  frame_->nex_ = temp;
  temp->pre_ = frame_;
  frame_->pre_ = head;
}
}  // namespace bustub
