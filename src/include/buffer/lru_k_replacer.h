//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// lru_k_replacer.h
//
// Identification: src/include/buffer/lru_k_replacer.h
//
// Copyright (c) 2015-2022, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#pragma once

#include <limits>
#include <list>
#include <memory>
#include <mutex>  // NOLINT
#include <unordered_map>
#include <vector>

#include "common/config.h"
#include "common/macros.h"

namespace bustub {

/**
 * LRUKReplacer implements the LRU-k replacement policy.
 *
 * The LRU-k algorithm evicts a frame whose backward k-distance is maximum
 * of all frames. Backward k-distance is computed as the difference in time between
 * current timestamp and the timestamp of kth previous access.
 *
 * A frame with less than k historical references is given
 * +inf as its backward k-distance. When multiple frames have +inf backward k-distance,
 * classical LRU algorithm is used to choose victim.
 * 课程页面说的是用FIFO 这里是错的！！！,数据也是FIFO
 */
class LRUKReplacer {
 public:
  /**
   *
   * TODO(P1): Add implementation
   *
   * @brief a new LRUKReplacer.
   * @param num_frames the maximum number of frames the LRUReplacer will be required to store
   */
  explicit LRUKReplacer(size_t num_frames, size_t k);

  DISALLOW_COPY_AND_MOVE(LRUKReplacer);
  // ？宏定义了一堆，删除这个名字？是C++有自带的LRU-k吗？不懂

  /**
   * TODO(P1): Add implementation
   *
   * @brief Destroys the LRUReplacer.
   */
  ~LRUKReplacer() = default;

  /**
   * TODO(P1): Add implementation
   *
   * @brief Find the frame with largest backward k-distance and evict that frame. Only frames
   * that are marked as 'evictable' are candidates for eviction.
   *
   * A frame with less than k historical references is given +inf as its backward k-distance.
   * If multiple frames have inf backward k-distance, then evict the frame with the earliest
   * timestamp overall.
   *
   * Successful eviction of a frame should decrement the size of replacer and remove the frame's
   * access history.
   *
   * @param[out] frame_id id of frame that is evicted.
   * @return true if a frame is evicted successfully, false if no frames can be evicted.
   */
  auto Evict(frame_id_t *frame_id) -> bool;

  /**
   * TODO(P1): Add implementation
   *
   * @brief Record the event that the given frame id is accessed at current timestamp.
   * Create a new entry for access history if frame id has not been seen before.
   *
   * If frame id is invalid (ie. larger than replacer_size_), throw an exception. You can
   * also use BUSTUB_ASSERT to abort the process if frame id is invalid.
   *
   * @param frame_id id of frame that received a new access.
   */
  void RecordAccess(frame_id_t frame_id);

  /**
   * TODO(P1): Add implementation
   *
   * @brief Toggle whether a frame is evictable or non-evictable. This function also
   * controls replacer's size. Note that size is equal to number of evictable entries.
   *
   * If a frame was previously evictable and is to be set to non-evictable, then size should
   * decrement. If a frame was previously non-evictable and is to be set to evictable,
   * then size should increment.
   *
   * If frame id is invalid, throw an exception or abort the process.
   *
   * For other scenarios, this function should terminate without modifying anything.
   *
   * @param frame_id id of frame whose 'evictable' status will be modified
   * @param set_evictable whether the given frame is evictable or not
   */
  void SetEvictable(frame_id_t frame_id, bool set_evictable);

  /**
   * TODO(P1): Add implementation
   *
   * @brief Remove an evictable frame from replacer, along with its access history.
   * This function should also decrement replacer's size if removal is successful.
   *
   * Note that this is different from evicting a frame, which always remove the frame
   * with largest backward k-distance. This function removes specified frame id,
   * no matter what its backward k-distance is.
   *
   * If Remove is called on a non-evictable frame, throw an exception or abort the
   * process.
   *
   * If specified frame is not found, directly return from this function.
   *
   * @param frame_id id of frame to be removed
   */
  void Remove(frame_id_t frame_id);

  /**
   * TODO(P1): Add implementation
   *
   * @brief Return replacer's size, which tracks the number of evictable frames.
   *
   * @return size_t
   */
  auto Size() -> size_t;

  class Frame {
   public:
    frame_id_t frame_id_;
    std::weak_ptr<Frame> pre_, nex_;
    std::list<size_t> timestamps_;  // 记录时间戳序列,因为这里不可能无限制一直写下去
    // 超过K个之后前面必须要清空，还可以记录一个record_cnt_ 记录被访问几次了
    size_t record_cnt_{0};
    // bool atwhere{false}; // 记录在哪个队列里面，false就在history队列，否则就是在LRU队列
    bool evictable_{true};
    Frame() = default;
    explicit Frame(frame_id_t frame_id) : frame_id_(frame_id) {}
  };

  // 自己加的函数
  void RemoveFromList(const std::shared_ptr<Frame> &frame_);  // 这里应该不需要区分在那个链表里面，只要移除了就行
  void InsertToListHead(const std::shared_ptr<Frame> &frame_, const std::shared_ptr<Frame> &head);

 private:
  // TODO(student): implement me! You can replace these member variables as you like.
  // Remove maybe_unused if you start using them.
  size_t current_timestamp_{0};
  size_t curr_size_{0};
  size_t replacer_size_;
  size_t k_;
  std::mutex latch_;

  // 两个链表，一个记录history，使用次数，一个是 双向链表(手动模拟)+hashmap记录,模拟驱逐队列
  std::shared_ptr<Frame> lru_head_, lru_tail_;          // LRU 链表
  std::shared_ptr<Frame> history_head_, history_tail_;  // 历史记录，也就是不到k次的那些结点
  // 两个链表都采用手动模拟形式，而且历史记录链表 也采用LRU算法驱逐，不用FIFO
  // tail是最先被驱逐位置，头是新位置

  // std::shared_ptr<Frame> L(new Frame());
  // std::shared_ptr<int> test(new int(100));
  // 这个直接用new出来的裸指针直接初始化不知道为什么在这里是报错的，所以选择了在另外一个文件里面
  // 构造函数那里用makeshared初始化

  size_t tot_size_{0};  // 本来这个感觉不用加，但是因为设置是否可以驱逐那里要求curr_size要变大小
  // 这样，curr_size就是可以被驱逐大小的记录,那里的定义明确写着就是这个可以驱逐个数代表size
  // 但是插入看有没有空是看两个链表除去哨兵结点的 大小和初始化分配Frame个数来比较的。
  // 所以我tot_size_记录的是两个链表的全部大小。

  std::unordered_map<frame_id_t, std::shared_ptr<Frame>> mp_;
  // 页面和结点指针的映射关系，方便找到对应结点，类似于数组用的 node[id]
};  // 总的来说，比LRU 项目多了，设置是否可以驱逐+ -k的实现
}  // namespace bustub
