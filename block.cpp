#include "block.h"
#include "list.h"

Block::Block() : allocate_from(nullptr), allocate_size(0) {};
void *Block::data() { return reinterpret_cast<char *>((this)) + sizeof(*this); }

ListNode *Block::transmute() {
  auto node = reinterpret_cast<ListNode *>(this);
  node->prev = nullptr;
  node->next = nullptr;
  return node;
}
