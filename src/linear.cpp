#include "../head/linear.h"
#include <iostream>

LinearAllocator::LinearAllocator(Buddy_allocation &buddy_ref, size_t pageSize)
    : buddy(buddy_ref), page_size(pageSize) {}

LinearAllocator::~LinearAllocator() { reset(); }

void *LinearAllocator::allocate(size_t size) {
  // Pagina existente
  if (!pages.empty()) {
    LinearPage &lasPage = pages.back();
    size_t aligned_size = (size + 3) & ~3;

    if (lasPage.used_offset + aligned_size <= lasPage.total_size) {
      void *ptr = static_cast<char *>(lasPage.base_ptr) + lasPage.used_offset;
      lasPage.used_offset += aligned_size;
      return ptr;
    }
  }

  // Pedir una nueva pagina
  size_t new_page_req = std::max(page_size, size);
  void *new_block = buddy.malloc(new_page_req);
  if (!new_block)
    return nullptr;

  pages.emplace_back(new_block, new_page_req);
  LinearPage &new_page = pages.back();
  size_t aligned_size = (size + 3) & ~3;
  void *ptr = new_page.base_ptr;
  new_page.used_offset += aligned_size;

  std::cout << "[LinearAllocator] Nueva Pagina creada " << new_page_req << '\n';
  return ptr;
}

bool LinearAllocator::owns(void *ptr) const {
  for (const auto &page : pages) {
    if (ptr <= page.base_ptr &&
        ptr < static_cast<char *>(page.base_ptr) + page.total_size)
      return true;
  }
  return false;
}

void LinearAllocator::reset() {
  for (auto &page : pages) {
    buddy.free(page.base_ptr);
  }
  pages.clear();
}
