#include "../head/slab.h"
#include <cstddef>
#include <iostream>

Slab::Slab(size_t objectSize, size_t poolSize) {
  object_size = objectSize;
  pool_limit = poolSize;

  memory_pool = new char[pool_limit];
  size_t num_objects = pool_limit / object_size;
  for (size_t i = 0; i < num_objects; i++)
    free_list.push_back(memory_pool + (i * object_size));
}

Slab::~Slab() { delete[] memory_pool; }

bool Slab::owns(void *ptr) const {
  char *p = static_cast<char *>(ptr);
  return (p >= memory_pool) && (p < memory_pool + pool_limit);
}

void *Slab::allocate() {
  if (free_list.empty())
    return nullptr;
  void *ptr = free_list.back();
  free_list.pop_back();
  return ptr;
}

void Slab::free(void *ptr) { free_list.push_back(ptr); }

SlabAllocator::SlabAllocator()
    : slab_32(32, 1024 * 1024), slab_64(64, 1024 * 1024),
      slab_128(128, 1024 * 1024), slab_256(256, 1024 * 1024) {}

void *SlabAllocator::allocate(size_t size) {
  if (size <= 32)
    return slab_32.allocate();
  if (size <= 64)
    return slab_64.allocate();
  if (size <= 128)
    return slab_128.allocate();
  if (size <= 256)
    return slab_256.allocate();
  return nullptr;
}

bool SlabAllocator::free(void *ptr) {
  if (slab_32.owns(ptr)) {
    slab_32.free(ptr);
    return true;
  }
  if (slab_64.owns(ptr)) {
    slab_64.free(ptr);
    return true;
  }
  if (slab_128.owns(ptr)) {
    slab_128.free(ptr);
    return true;
  }
  if (slab_256.owns(ptr)) {
    slab_256.free(ptr);
    return true;
  }
  return false;
}
