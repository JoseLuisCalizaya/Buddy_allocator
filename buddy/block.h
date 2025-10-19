#pragma once

#ifndef BLOCK_H
#define BLOCK_H

#include "list.h"
#include <cstddef>

class Block {
public:
  size_t allocate_size = 0;
  void *allocate_from = nullptr;
  Block();
  void *data();
  class ListNode *transmute();
};

#endif // BLOCK_H
