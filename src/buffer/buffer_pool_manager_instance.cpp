//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// buffer_pool_manager_instance.cpp
//
// Identification: src/buffer/buffer_pool_manager.cpp
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "buffer/buffer_pool_manager_instance.h"

#include "common/exception.h"
#include "common/macros.h"

#include "common/logger.h"

namespace bustub {

BufferPoolManagerInstance::BufferPoolManagerInstance(size_t pool_size, DiskManager *disk_manager, size_t replacer_k,
                                                     LogManager *log_manager)
    : pool_size_(pool_size), disk_manager_(disk_manager), log_manager_(log_manager) {
  // we allocate a consecutive memory space for the buffer pool
  pages_ = new Page[pool_size_];
  // 所有页框的集合，不是指页面，别被pages迷惑了！！！

  page_table_ = new ExtendibleHashTable<page_id_t, frame_id_t>(bucket_size_);
  replacer_ = new LRUKReplacer(pool_size, replacer_k);

  // Initially, every page is in the free list.
  for (size_t i = 0; i < pool_size_; ++i) {
    free_list_.emplace_back(static_cast<int>(i));
  }

  // 所有变量都初始化了，那这里应该没有什么要再动的，最多自己加变量的时候需要初始化一下

  // TODO(students): remove this line after you have implemented the buffer pool manager
  // throw NotImplementedException(
  //     "BufferPoolManager is not implemented yet. If you have finished implementing BPM, please remove the throw "
  //     "exception line in `buffer_pool_manager_instance.cpp`.");
}

BufferPoolManagerInstance::~BufferPoolManagerInstance() {
  delete[] pages_;
  delete page_table_;
  delete replacer_;
  // 都是普通指针，所以需要内存释放
}

auto BufferPoolManagerInstance::NewPgImp(page_id_t *page_id) -> Page * {
  std::scoped_lock<std::mutex> lock(latch_);
  frame_id_t new_frame_id;
  if (!free_list_.empty()) {  // 分配页框号：有空闲页框就直接用，否则驱逐一个
    new_frame_id = free_list_.front();
    free_list_.pop_front();
  } else {
    bool st = replacer_->Evict(&new_frame_id);
    if (!st) {
      page_id = nullptr;
      return nullptr;
    }
  }

  auto new_page_id = AllocatePage();  // 分配页面号，这里是指进程往后写内容，页面是越来越多的
  // 页面个数是没有上限的！！！

  // 页框要是脏的，先写回
  Page *new_frame = &pages_[new_frame_id];  // 用指针获取对应的页框，而不是用对象获取
  // 参数里面pages[] 应该指的是页框，而不是实际页面
  // 暂时就这里的page 和页框是不同于理解的，后面不同的地方再指出来

  if (new_frame->IsDirty()) {
    // 这里的id 指的是页框上放的页面号，和我自己定义的是一个类型，但是
    // 这里的页面id是页框上面旧的页面号，所以需要把这个旧的页面号上的内容写回去
    disk_manager_->WritePage(new_frame->GetPageId(), new_frame->GetData());
    new_frame->is_dirty_ = false;
  }

  // 分配号码和处理脏数据搞定后，维护一下信息即可。
  // 页面到页框的映射用手写hash表
  // 页框到页面映射用页框里面的getpageid函数
  page_table_->Remove(new_frame->GetPageId());  // 两行维护手写拓展hash表上的页面和页框映射关系
  new_frame->ResetMemory();  // 三行维护页框上面信息，包括数据清空，页面号设置，和pincount置1
  // 这里清空旧页面，不太好调用下面的remove函数，因为evict函数里面replacer里面的操作也已做了一半了
  // 而且，free_list也要删了再拿出来，不太方便，不如直接写

  page_table_->Insert(new_page_id, new_frame_id);
  new_frame->page_id_ = new_page_id;
  new_frame->pin_count_ = 1;
  replacer_->RecordAccess(new_frame_id);         // LRU-k淘汰算法的记录修改，
  replacer_->SetEvictable(new_frame_id, false);  // 按要求设置页面不能被驱逐
  // 现在一个问题是，我上面为了拿到驱逐页框号，已经调用了一次evict ，但是这个记录再打进去，那不是还得重新驱逐一次
  // 哦，想通了，要是我直接调用LRU-k记录(某个页框)函数的话，才会新建的时候发生驱逐
  // 但是我现在是先驱逐了一个之后，里面两个链表大小肯定没满了，刚好空了一个位置，所以，一定不发生驱逐，
  // 所以这里再记录就不用担心重复驱逐的问题，

  // 还有一个问题是lru-k里面record_cnt_的维护，应该不需要处理了
  // 因为要是新点不用担心，驱逐之后的点的话也没问题，因为是先驱逐，结点会被清空
  // 重新建立，这里记录record_cnt相当于被清空了

  *page_id = new_page_id;  // 差点忘记传回 对应的页面号了。这里页面号应该是指进程的页面号
  // 可以用的页面空间
  return new_frame;  // 新建页面，返回一个页框号，表示新的页面建在了哪个页框上面。
}

auto BufferPoolManagerInstance::FetchPgImp(page_id_t page_id) -> Page * {
  // 这个应该核心函数了吧？？？需要判断页面内容在内存还是在磁盘
  // 先判断一下页面号是否非法，其实应该不需要写
  // 我加了这大于等于下一个分配号的判断，要是页面那里修改了分配规则，这里需要修改！！！
  std::scoped_lock<std::mutex> lock(latch_);
  if (page_id == INVALID_PAGE_ID || page_id >= next_page_id_) {
    LOG_INFO("非法页面号！！！");
    return nullptr;
  }
  frame_id_t frame_id;
  if (!page_table_->Find(page_id, frame_id)) {  // 页框里面找不到页面,就去想办法找到一个页框
    if (!free_list_.empty()) {
      frame_id = free_list_.front();
      free_list_.pop_front();
    } else {
      bool st = replacer_->Evict(&frame_id);
      if (!st) {
        return nullptr;
      }
    }
    Page *frame = &pages_[frame_id];

    if (frame->IsDirty()) {
      disk_manager_->WritePage(frame->GetPageId(), frame->GetData());
      frame->is_dirty_ = false;
    }
    page_table_->Remove(frame->GetPageId());
    frame->ResetMemory();
    disk_manager_->ReadPage(page_id, frame->data_);  // 比新建唯一多一步就是写入内容
    page_table_->Insert(page_id, frame_id);
    frame->page_id_ = page_id;
    frame->pin_count_ = 1;
    replacer_->RecordAccess(frame_id);
    replacer_->SetEvictable(frame_id, false);
    return frame;
  }
  // 下面是内存里面找的到页面
  Page *frame = &pages_[frame_id];
  replacer_->RecordAccess(frame_id);
  replacer_->SetEvictable(frame_id, false);
  frame->pin_count_++;

  return frame;
}

auto BufferPoolManagerInstance::UnpinPgImp(page_id_t page_id, bool is_dirty) -> bool {
  std::scoped_lock<std::mutex> lock(latch_);
  if (page_id == INVALID_PAGE_ID) {
    return false;
  }
  frame_id_t frame_id;
  if (!page_table_->Find(page_id, frame_id)) {
    return false;
  }
  Page *frame = &(pages_[frame_id]);
  if (frame->GetPinCount() == 0) {
    return false;
  }
  frame->pin_count_--;
  if (frame->GetPinCount() == 0) {  // 修改pincount 并且维护是否可以驱逐
    replacer_->SetEvictable(frame_id, true);
  }
  // 这个维护dirty没看懂再干啥
  frame->is_dirty_ |= is_dirty;  // 这里看了别人的，是要 传入是1，给他改成1
  // 传入0，保持原有标记
  return true;
}

auto BufferPoolManagerInstance::FlushPgImp(page_id_t page_id) -> bool {
  std::scoped_lock<std::mutex> lock(latch_);
  if (page_id == INVALID_PAGE_ID) {
    return false;
  }
  frame_id_t frame_id;
  if (!page_table_->Find(page_id, frame_id)) {
    return false;
  }
  disk_manager_->WritePage(page_id, pages_[frame_id].GetData());
  pages_[frame_id].is_dirty_ = false;
  return true;
}

void BufferPoolManagerInstance::FlushAllPgsImp() {
  for (size_t i = 0; i < pool_size_; i++) {
    FlushPgImp(pages_[i].GetPageId());
  }
}

auto BufferPoolManagerInstance::DeletePgImp(page_id_t page_id) -> bool {
  std::scoped_lock<std::mutex> lock(latch_);
  if (page_id == INVALID_PAGE_ID) {
    LOG_INFO("删除了-1页面");
    return true;
  }
  frame_id_t frame_id;
  if (!page_table_->Find(page_id, frame_id)) {
    return true;
    // 要求里面删除不存在的页面视为已经删除了，所以返回true而不是false
  }
  Page *frame = &(pages_[frame_id]);
  if (frame->GetPinCount() > 0) {
    return false;  // pincount 应该就是几个进程要求这个页面锁死，不能被调出
  }

  // 删除操作
  frame->ResetMemory();
  replacer_->Remove(frame_id);
  page_table_->Remove(page_id);
  free_list_.push_back(frame_id);

  DeallocatePage(page_id);  // 这里应该是把页面重新放回池子里面，但是这个不需要要实现，因为需要数据结构维护页面
  return true;
}

auto BufferPoolManagerInstance::AllocatePage() -> page_id_t { return next_page_id_++; }

}  // namespace bustub
