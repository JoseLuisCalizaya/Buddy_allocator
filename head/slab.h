#pragma once
#ifndef SLAB_H
#define SLAB_H

#include <cstddef>
#include <cstdint>
#include <vector>

class Slab {
public:
  Slab(size_t objectSize, size_t poolSize);
  ~Slab();
  void *allocate();
  void free();
  bool owns(void *ptr) const;

private:
  size_t object_size;
  char *memory_pool;
  size_t pool_limit;
  std::vector<void *> free_list;
};

class SlabAllocator {
public:
  SlabAllocator();
  void *allocate(size_t size);
  bool free(void *ptr);

private:
  Slab slab_32;
  Slab slab_64;
  Slab slab_128;
  Slab slab_256;
};

#endif // SLAB_H
