#include "buddy.h"
#include "list.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <sys/types.h>

Buddy_allocation::Buddy_allocation() {

  heap_base = new char[k_size]();
  size_t num_min_blocks = k_size / Min_alloc;
  metadata_orders = new uint8_t[num_min_blocks]();

  ListNode *root = reinterpret_cast<ListNode *>(heap_base);
  free_lists[k_maximum_order].push(root);
}

Buddy_allocation::~Buddy_allocation() {
  delete[] heap_base;
  delete[] metadata_orders;
}

Buddy_allocation::set_order(void *ptr, uint8_t order) {
  size_t offset = static_cast<char *>(ptr) - heap_base;
  size_t index = offset / Min_alloc;
  metadata_orders[index] = order;
}

Buddy_allocation::get_order(void *ptr) {
  size_t offset = static_cast<char *>(ptr) - heap_base;
  size_t index = offset / Min_alloc;
  return metadata_orders[index];
}
void *Buddy_allocation::malloc(const size_t request) {

  if (request == 0 || request > k_size)
    return nullptr;

  const size_t r_size = std::max(request, (size_t)Min_alloc);

  size_t required_order = 0;
  size_t size_for_order = Min_alloc;
  while (size_for_order < r_size) {
    size_for_order <<= 1; // *2
    required_order++;
  }
  // Si aun con el mayor orden no hay sitio, falla
  if (required_order > k_maximum_order)
    return nullptr;

  // Buscar bloque de memoria libre
  size_t order = required_order;
  ListNode *node = nullptr;
  for (; order <= k_maximum_order; ++order) {
    node = free_lists[order].pop();
    if (node)
      break;
  }

  if (!node)
    return nullptr;

  // split
  auto index = index_to_node(node, order);
  if (order < k_maximum_order) {
    to_split(parent(index));
  }

  while (order > required_order) {
    const auto index_here = index_to_node(node, order);
    to_split(index_here);
    order--;

    auto right = node_to_index(child_right(index_here), order);
    free_lists[order].push(right);

    node = node_to_index(child_left(index_here), order);
  }
  // Guardar metadatos externamente
  void *ptr = reinterpret_cast<void *>(node);
  set_order(ptr, (uint8_t)required_order);
  return ptr;
}

void Buddy_allocation::free(void *ptr) {
  if (!ptr)
    return;

  // recuperacion de metadata
  uint8_t stored_order = get_order(ptr);
  size_t size = (size_t)Min_alloc << stored_order;

  if ((char *)ptr < heap_base || (char *)ptr >= heap_base + k_size)
    return;

  size_t order = stored_order;
  ListNode *node = reinterpret_cast<ListNode *>(ptr);
  auto index = index_to_node(node, order);

  while (order < k_maximum_order && can_split(parent(index))) {
    auto sibling_node = node_to_index(sibling(index), order);
    sibling_node->remove();
    index = parent(index);
    to_split(index);
    order++;
    node = node_to_index(index, order);
  }

  free_lists[order].push(node);
  if (order < k_maximum_order)
    to_split(parent(index));
}

size_t Buddy_allocation::index_to_node(ListNode *node, size_t order) {
  const auto tree_depth = k_maximum_order - order;
  const auto first_index = (1 << tree_depth) - 1;
  const auto block_size = (1 << order) * Min_alloc;
  return first_index +
         (reinterpret_cast<char *>(node) - heap_base) / block_size;
}

ListNode *Buddy_allocation::node_to_index(size_t index, size_t order) {
  const auto tree_depth = k_maximum_order - order;
  const auto first_index = (1 << tree_depth) - 1;
  const auto block_size = (1 << order) * Min_alloc;
  return reinterpret_cast<ListNode *>(heap_base +
                                      (index - first_index) * block_size);
}

size_t Buddy_allocation::parent(size_t i) const { return (i - 1) / 2; }
size_t Buddy_allocation::child_left(size_t i) const { return 2 * i + 1; }
size_t Buddy_allocation::child_right(size_t i) const { return 2 * i + 2; }
size_t Buddy_allocation::sibling(size_t i) const { return ((i - 1) ^ 1) + 1; }
bool Buddy_allocation::can_split(size_t i) const {
  return (split_nodes[i / 8] >> (i % 8)) & 1;
}

void Buddy_allocation::to_split(size_t i) {
  split_nodes[i / 8] ^= (1 << (i % 8));
}
