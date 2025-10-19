#include "buddy.h"
#include "list.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdlib>

Buddy_allocation::Buddy_allocation() {
  ListNode *root = reinterpret_cast<ListNode *>(heap_base);
  free_lists[k_maximum_order].push(root);
}

void *Buddy_allocation::malloc(const size_t request) {
  if (request == 0)
    return nullptr;
  if (request > SIZE_MAX - sizeof(Block))
    return nullptr;

  const size_t required_size_unrounded = request + sizeof(Block);
  const size_t r_size = std::max(required_size_unrounded, (size_t)Min_alloc);
  if (r_size > k_size)
    return nullptr;

  size_t required_order = 0;
  size_t size_for_order = Min_alloc;
  while (size_for_order < r_size) {
    if (required_order >= k_maximum_order)
      break;
    size_for_order <<= 1; // *2
    required_order++;
  }
  // Si aun con el mayor orden no hay sitio, falla
  if (size_for_order < r_size) {
    return nullptr;
  }
  // 5) Buscar el primer free_list que tenga un bloque (como hacías)
  size_t order = required_order;
  ListNode *node = nullptr;
  for (; order <= k_maximum_order; ++order) {
    node = free_lists[order].pop();
    if (node)
      break;
  }

  if (!node) {
    return nullptr;
  }

  auto index = index_to_node(node, order);
  if (order < k_maximum_order) {
    to_split(parent(index));
  }

  // Splitting mientras sea necesario
  while (order > required_order) {
    const auto index_here = index_to_node(node, order);
    to_split(index_here);
    order--;

    auto right = node_to_index(child_right(index_here), order);
    free_lists[order].push(right);

    node = node_to_index(child_left(index_here), order);
  }

  // 7) Finalizar: marcar y devolver
  auto block = node->transmute();
  block->allocate_size = size_for_order; // usar el tamaño real para el order
  block->allocate_from = __builtin_return_address(0);
  return block->data();
}

void Buddy_allocation::free(void *ptr) {
  if (!ptr)
    return;
  Block *block =
      reinterpret_cast<Block *>(reinterpret_cast<char *>(ptr) - sizeof(Block));
  assert(block->allocate_from != nullptr);
  assert(block->allocate_size <= Buddy_allocation::k_size);

  auto order = log2(block->allocate_size) - log2(Min_alloc);
  ListNode *node = block->transmute();
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
