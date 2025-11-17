#pragma once

#include <cstddef>
#include <limits>
#ifndef BUDDY_H
#define BUDDY_H

#include "block.h"
#include "list.h"
#include <algorithm>
#include <cstdint>
#include <iterator>
constexpr size_t Min_alloc = std::max(sizeof(Block), sizeof(ListNode));
constexpr size_t log2(const size_t n) {
  size_t r = 0;
  size_t m = 1;
  while (n > m) {
    r++;
    if (m > std::numeric_limits<size_t>::max() / 2)
      return r;
    m *= 2;
  }
  return r;
}
class Buddy_allocation {
public:
  static constexpr size_t k_size = 64 * 1024 * 1024;
  void *malloc(const size_t);
  void free(void *);
  Buddy_allocation();
  ~Buddy_allocation();
  alignas(std::max_align_t) char *heap_base = nullptr;

private:
  static constexpr size_t k_maximum_order = log2(k_size) - log2(Min_alloc);
  ListNode free_lists[k_maximum_order + 1] = {};
  uint8_t split_nodes[(1 << k_maximum_order) / 8] = {};
  size_t index_to_node(ListNode *, size_t);
  ListNode *node_to_index(size_t, size_t);
  size_t parent(size_t) const;
  size_t child_left(size_t) const;
  size_t child_right(size_t) const;
  size_t sibling(size_t) const;
  bool can_split(size_t) const;
  void to_split(size_t);
};

#endif // BUDDY_H
