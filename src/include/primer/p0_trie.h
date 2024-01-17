//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// p0_trie.h
//
// Identification: src/include/primer/p0_trie.h
//
// Copyright (c) 2015-2022, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#pragma once
//可以使用 #pragma once 来确保头文件只被编译一次。和#ifndef差不多
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
//上面是一些标准库

#include "common/exception.h"
#include "common/rwlatch.h"
//看了好多都是从include为路径开始，包括include以外的文件内的include

namespace bustub {

/**
 * TrieNode is a generic container for any node in Trie. 普通结点
 */
class TrieNode {
  public:   //访问修饰符
  /**
   * TODO(P0): Add implementation
   *  //brief是概要信息，这里应该是介绍对应实体的功能，即任务要求。param是参数信息
   * @brief Construct a new Trie Node object with the given key char.    
   * is_end_ flag should be initialized to false in this constructor.
   *
   * @param key_char Key character of this trie node
   */
  explicit TrieNode(char key_char) {//重载构造函数+explicit修饰
      this->is_end_=false;
      this->key_char_=key_char;
      // this->children_.clear();//初始化这个hashmap，而不是去new出这个变量，但实际上这个本来就是空的
      //所以应该是不需要clear的
  }


  //这里重载了构造函数，但是没处理默认构造函数，应该是别的地方用不到这个默认构造函数
  //我这里选择加上吧
  // TrieNode()=default;

  /**
   * TODO(P0): Add implementation
   *
   * @brief Move constructor for trie node object. The unique pointers stored
   * in children_ should be moved from other_trie_node to new trie node.
   * 
   * @param other_trie_node Old trie node.
   */
  TrieNode(TrieNode &&other_trie_node) noexcept {
    this->is_end_=other_trie_node.is_end_;
    this->key_char_=other_trie_node.key_char_;
    this->children_=std::move(other_trie_node.children_);// 这里hashmap复制,应该用move，而不是直接复制一份 
  }

  /**
   * @brief Destroy the TrieNode object.
   */
  virtual ~TrieNode() = default;

  /**
   * TODO(P0): Add implementation
   *
   * @brief Whether this trie node has a child node with specified key char.
   *  
   * 查找用mp[?] 与 mp.find
   * 
   * @param key_char Key char of child node.
   * @return True if this trie node has a child with given key, false otherwise.
   */
  bool HasChild(char key_char) const { 
    if(this->children_.find(key_char)!=this->children_.end()){
      return true;
    }
    else return false;
  }

  /**
   * TODO(P0): Add implementation
   *
   * @brief Whether this trie node has any children at all. This is useful
   * when implementing 'Remove' functionality.
   *
   * @return True if this trie node has any child node, false if it has no child node.
   */
  bool HasChildren() const { 
    if(!this->children_.empty())return true;
    else return false;
  }

  /**
   * TODO(P0): Add implementation
   *
   * @brief Whether this trie node is the ending character of a key string.
   *
   * @return True if is_end_ flag is true, false if is_end_ is false.
   */
  bool IsEndNode() const { 
    return this->is_end_;
  }

  /**
   * TODO(P0): Add implementation
   *
   * @brief Return key char of this trie node.
   *
   * @return key_char_ of this trie node.
   */
  char GetKeyChar() const { 
    return this->key_char_;
  }

  /**
   * TODO(P0): Add implementation
   *
   * @brief Insert a child node for this trie node into children_ map, given the key char and
   * unique_ptr of the child node. If specified key_char already exists in children_,
   * return nullptr. If parameter `child`'s key char is different than parameter
   * `key_char`, return nullptr.
   *
   * Note that parameter `child` is rvalue and should be moved when it is
   * inserted into children_map.
   *
   * The return value is a pointer to unique_ptr because pointer to unique_ptr can access the
   * underlying data without taking ownership of the unique_ptr. Further, we can set the return
   * value to nullptr when error occurs.
   *
   * @param key Key of child node
   * @param child Unique pointer created for the child node. This should be added to children_ map.
   * @return Pointer to unique_ptr of the inserted child node. If insertion fails, return nullptr.
   */
  std::unique_ptr<TrieNode> *InsertChildNode(char key_char, std::unique_ptr<TrieNode> &&child) { 
      if(!HasChild(key_char)){
        // this->children_.insert(std::make_pair(key_char,child));   //  要改观了！！！颠覆了之前的认识了！！！
        // this->children_.insert({key_char,child});
        // this->children_[key_char]=child;    //下面这种才行 why？
        children_[key_char]=std::move(child);
        return &children_[key_char];
      }
      else return nullptr;
   }

  /**
   * TODO(P0): Add implementation
   *
   * @brief Get the child node given its key char. If child node for given key char does
   * not exist, return nullptr.
   *
   * @param key Key of child node
   * @return Pointer to unique_ptr of the child node, nullptr if child
   *         node does not exist.
   */
  std::unique_ptr<TrieNode> *GetChildNode(char key_char) { //指针问题，大问题！！！
    if(this->HasChild(key_char)){
      return &children_.find(key_char)->second;
    }
    else return nullptr;
  }

  /**
   * TODO(P0): Add implementation
   *
   * @brief Remove child node from children_ map.
   * If key_char does not exist in children_, return immediately.
   *
   * @param key_char Key char of child node to be removed
   */
  void RemoveChildNode(char key_char) {
    if(this->HasChild(key_char)){
      children_.erase(children_.find(key_char)); //删除 erase(find(key))
    }
    else return ;
  }

  /**
   * TODO(P0): Add implementation
   *
   * @brief Set the is_end_ flag to true or false.
   *
   * @param is_end Whether this trie node is ending char of a key string
   */
  void SetEndNode(bool is_end) {
    is_end_=is_end;
  }

protected:
  /** Key character of this trie node */
  char key_char_;
  /** whether this node marks the end of a key */
  bool is_end_{false};
  /** A map of all child nodes of this trie node, which can be accessed by each
   * child node's key char. */
  std::unordered_map<char, std::unique_ptr<TrieNode>> children_;
};

/**
 * TrieNodeWithValue is a node that mark the ending of a key, and it can
 * hold a value of any type T.
 */
template <typename T>
class TrieNodeWithValue : public TrieNode {
 private:
  /* Value held by this trie node. */
  T value_;

 public:
  /**
   * TODO(P0): Add implementation
   *
   * @brief Construct a new TrieNodeWithValue object from a TrieNode object and specify its value.
   * This is used when a non-terminal TrieNode is converted to terminal TrieNodeWithValue.
   *
   * The children_ map of TrieNode should be moved to the new TrieNodeWithValue object.
   * Since it contains unique pointers, the first parameter is a rvalue reference.
   *
   * You should:
   * 1) invoke TrieNode's move constructor to move data from TrieNode to
   * TrieNodeWithValue.
   * 2) set value_ member variable of this node to parameter `value`.
   * 3) set is_end_ to true
   *
   * @param trieNode TrieNode whose data is to be moved to TrieNodeWithValue
   * @param value
   */
  TrieNodeWithValue(TrieNode &&trieNode, T value) {
    this->key_char_=trieNode->key_char_;
    this->children_=trieNode->children_;
    this->is_end_=true;
    this->value_=value;

    //这里可以用继承来减少代码量
  }

  /**
   * TODO(P0): Add implementation
   *
   * @brief Construct a new TrieNodeWithValue. This is used when a new terminal node is constructed.
   *
   * You should:
   * 1) Invoke the constructor for TrieNode with given key_char.
   * 2) Set value_ for this node.
   * 3) set is_end_ to true.
   *
   * @param key_char Key char of this node
   * @param value Value of this node
   */
  TrieNodeWithValue(char key_char, T value) {
    this->key_char_=key_char;
    this->children_.clear();
    this->is_end_=true;
    this->value_=value;
  }

  /**
   * @brief Destroy the Trie Node With Value object
   */
  ~TrieNodeWithValue() override = default;

  /**
   * @brief Get the stored value_.
   *
   * @return Value of type T stored in this node
   */
  T GetValue() const { 
    return this->value_; 
  }
};

/**
 * Trie is a concurrent key-value store. Each key is string and its corresponding
 * value can be any type.
 */
class Trie {
 private:
  /* Root node of the trie */
  std::unique_ptr<TrieNode> root_;
  /* Read-write lock for the trie */
  ReaderWriterLatch latch_;   //读写锁(latch)

 public:
  /**
   * TODO(P0): Add implementation
   *
   * @brief Construct a new Trie object. Initialize the root node with '\0'
   * character.
   */
  Trie(){
    // std::unique_ptr<TrieNode> root(new TrieNode('\0'));
    // this->root_=std::move(root);
    //应该两种都可以吧
    root_=std::make_unique<TrieNode>(TrieNode('\0'));
  };

  /**
   * TODO(P0): Add implementation
   *
   * @brief Insert key-value pair into the trie.
   *
   * If key is empty string, return false immediately.
   *
   * If key alreadys exists, return false. Duplicated keys are not allowed and
   * you should never overwrite value of an existing key.
   *
   * When you reach the ending character of a key:
   * 1. If TrieNode with this ending character does not exist, create new TrieNodeWithValue
   * and add it to parent node's children_ map.
   * 2. If the terminal node is a TrieNode, then convert it into TrieNodeWithValue by
   * invoking the appropriate constructor.
   * 3. If it is already a TrieNodeWithValue,
   * then insertion fails and return false. Do not overwrite existing data with new data.
   *
   * You can quickly check whether a TrieNode pointer holds TrieNode or TrieNodeWithValue
   * by checking the is_end_ flag. If is_end_ == false, then it points to TrieNode. If
   * is_end_ == true, it points to TrieNodeWithValue.
   *
   * @param key Key used to traverse the trie and find correct node
   * @param value Value to be inserted
   * @return True if insertion succeeds, false if key already exists
   */
  template <typename T>
  bool Insert(const std::string &key, T value) {
    if(key.size()==0)return false;
    auto st=key.begin();
    auto ed=key.end();
    std::unique_ptr<TrieNode>*p = &root_;//暂时的移动指针
    for(char c:str){
      if((*p)->GetChildNode== nullptr){
        (*p)->InsertChildNode(c,std::make_unique<TrieNode>(TrieNode(c)));
        //当前结点p上c 字符处插入一个新结点，结点初始化为c字符
      }
      p=(*p)->GetChildNode(c);
    }
    
    if((*p)->IsEndNode){
      return false;
    }
    //插入完成 后，最后一个点是普通的点，现在合并1，2两种情况，将当前点转化为end结点
    *p=std::unique_ptr<TrieNode>(new TrieNodeWithValue<T>(std::move(*(p>get())),value));
    //*p 是一个实体，
    //
    return true;
  }

  /**
   * TODO(P0): Add implementation
   *
   * @brief Remove key value pair from the trie.
   * This function should also remove nodes that are no longer part of another
   * key. If key is empty or not found, return false.
   *
   * You should:
   * 1) Find the terminal node for the given key.
   * 2) If this terminal node does not have any children, remove it from its
   * parent's children_ map.
   * 3) Recursively remove nodes that have no children and is not terminal node
   * of another key.
   *
   * @param key Key used to traverse the trie and find correct node
   * @return True if key exists and is removed, false otherwise
   */
  bool Remove(const std::string &key) { 
    if (key.empty()) {
      return false;
    }

    std::unique_ptr<TrieNode> *node = &root_;

    for (auto &ch : key) {
      if ((*node)->GetChildNode(ch) == nullptr) {

        return false;
      }

      node = (*node)->GetChildNode(ch);
    }

   RemoveAux(key, 0, &root_);
    return true;


  }
  void RemoveAux(const std::string &key, size_t index, std::unique_ptr<TrieNode> *pre) {
    if (index == key.size()) {
      return;
    }

    RemoveAux(key, index + 1, (*pre)->GetChildNode(key[index]));

    std::unique_ptr<TrieNode> *node = (*pre)->GetChildNode(key[index]);
    if (node == nullptr) {
      return;
    }

    if (!(*node)->HasChildren()) {
      (*pre)->RemoveChildNode(key[index]);
    }
  }

  /**
   * TODO(P0): Add implementation
   *
   * @brief Get the corresponding value of type T given its key.
   * If key is empty, set success to false.
   * If key does not exist in trie, set success to false.
   * If given type T is not the same as the value type stored in TrieNodeWithValue
   * (ie. GetValue<int> is called but terminal node holds std::string),
   * set success to false.
   *
   * To check whether the two types are the same, dynamic_cast
   * the terminal TrieNode to TrieNodeWithValue<T>. If the casted result
   * is not nullptr, then type T is the correct type.
   *
   * @param key Key used to traverse the trie and find correct node
   * @param success Whether GetValue is successful or not
   * @return Value of type T if type matches
   */
  template <typename T>
  T GetValue(const std::string &key, bool *success) {
 
    *success = false;
    std::unique_ptr<TrieNode> *node = &root_;

    for (auto &ch : key) {
      if ((*node)->GetChildNode(ch) == nullptr) {
    
        return {};
      }

      node = (*node)->GetChildNode(ch);
    }

    auto res = dynamic_cast<TrieNodeWithValue<T> *>(node->get());
    if (res == nullptr) {
  
      return {};
    }

    *success = true;
   
    return res->GetValue();
  
  }
};
}  // namespace bustub
