#include "list.h"
#include "block.h"
#include <cassert>
ListNode::ListNode() : prev(this), next(this) {};

void ListNode::push(ListNode *new_node) {
  assert(!new_node->prev && !new_node->next);
  prev->next = new_node;
  new_node->prev = prev;
  new_node->next = this;
  prev = new_node;
}

ListNode *ListNode::pop() {
  if (prev == this)
    return nullptr;
  ListNode *temp = prev;
  temp->remove();
  return temp;
}

void ListNode::remove() {
  assert(prev && next);
  prev->next = next;
  next->prev = prev;
  // no es necesario (o si?)
  prev = nullptr;
  next = nullptr;
}

Block *ListNode::transmute() {
  assert(!prev && !next);
  auto block = reinterpret_cast<Block *>(this);
  block->allocate_from = nullptr;
  block->allocate_size = 0;
  return block;
}
