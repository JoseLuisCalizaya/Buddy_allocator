#pragma once
#ifndef LINEAR_H
#define LINEAR_H

#include "buddy.h"
#include <cstddef>
#include <vector>

struct LinearPage {
  void *base_ptr;
  size_t total_size;
  size_t used_offset;

  LinearPage(void *ptr, size_t size)
      : base_ptr(ptr), total_size(size), used_offset(0) {}
};

class LinearAllocator {
public:
  LinearAllocator(Buddy_allocation &buddy_ref, size_t pageSize);
  ~LinearAllocator();

  void *allocate(size_t size);
  bool owns(void *ptr) const;
  void reset(); // Libera todas las páginas y reinicia

  size_t get_total_allocated() const;

private:
  Buddy_allocation &buddy;
  size_t page_size;
  std::vector<LinearPage> pages;
};

#endif // LINEAR_H
