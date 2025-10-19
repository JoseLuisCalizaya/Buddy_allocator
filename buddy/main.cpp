#include "block.h"
#include "buddy.h"
#include "list.h"
#include <cassert>
#include <iostream>
#include <random>
#include <set>
#include <vector>

void print_heap_info(Buddy_allocation &heap) {
  std::cout << "\n[Heap Info] Chunk size: " << Buddy_allocation::k_size
            << " bytes | Min allocation: " << Min_alloc << " bytes\n";
}

// Muestra información de una asignación
void log_allocation(size_t request, void *ptr) {
  if (ptr)
    std::cout << " Allocated " << request << " bytes at " << ptr << "\n";
  else
    std::cout << " Allocation of " << request << " bytes FAILED (OOM)\n";
}

// Muestra información de liberación
void log_free(void *ptr) { std::cout << "Freed block at " << ptr << "\n"; }

void test_lists() {
  std::cout << "\n=== Test: List Structure ===\n";
  ListNode list;
  ListNode elem_a, elem_b, elem_c;
  elem_a.prev = elem_b.prev = elem_c.prev = nullptr;
  elem_a.next = elem_b.next = elem_c.next = nullptr;

  list.push(&elem_a);
  assert(&elem_a == list.pop());
  assert(nullptr == list.pop());

  list.push(&elem_b);
  list.push(&elem_c);
  assert(&elem_c == list.pop());
  assert(&elem_b == list.pop());
  assert(nullptr == list.pop());

  list.push(&elem_b);
  list.push(&elem_c);
  list.push(&elem_a);
  elem_c.remove();
  assert(&elem_a == list.pop());
  assert(&elem_b == list.pop());
  assert(nullptr == list.pop());

  std::cout << "  ✅ List test passed\n";
}

void check_oom_after(Buddy_allocation &heap, size_t n, size_t size) {
  if (size <= sizeof(Block)) {
    std::cout << "Skipping invalid test" << size << '\n';
    return;
  }
  std::cout << "\n=== Test: OOM after " << n << " allocations of " << size
            << " bytes ===\n";

  std::set<void *> allocations;
  for (size_t i = 0; i < n; i++) {
    auto ptr = heap.malloc(size);
    log_allocation(size, ptr);
    assert(ptr != nullptr);

    const auto pair = allocations.insert(ptr);
    assert(pair.second);
  }

  auto ptr = heap.malloc(size);
  log_allocation(size, ptr);
  assert(ptr == nullptr);

  for (auto p : allocations)
    heap.free(p);

  std::cout << "  ✅ OOM test passed\n";
}

void stress_test() {
  std::cout << "\n=== Stress Test: Random Alloc/Free ===\n";
  constexpr size_t kSteps = 3000;

  struct Allocation {
    void *ptr = nullptr;
    size_t size = 0;
  };
  std::vector<Allocation> allocations;

  std::random_device rd;
  std::default_random_engine gen(rd());
  std::uniform_int_distribution<> action(0, 4);
  std::uniform_int_distribution<> small_alloc(1, 512);
  std::uniform_int_distribution<> medium_alloc(1024, 16384);
  std::uniform_int_distribution<> large_alloc(32768, 262144);

  Buddy_allocation heap;
  print_heap_info(heap);

  const auto test_allocation = [&](std::uniform_int_distribution<> &size_dist) {
    const size_t request = size_dist(gen);
    auto ptr = heap.malloc(request);
    log_allocation(request, ptr);

    if (ptr) {
      allocations.push_back({ptr, request});
    }
  };

  for (size_t step = 0; step < kSteps; step++) {
    const auto selection = action(gen);

    switch (allocations.empty() ? selection & 0x3 : selection) {
    case 0:
      test_allocation(small_alloc);
      break;
    case 1:
      test_allocation(medium_alloc);
      break;
    case 2:
      test_allocation(large_alloc);
      break;
    case 4:
      if (!allocations.empty()) {
        std::uniform_int_distribution<> index(0, allocations.size() - 1);
        const auto target = index(gen);
        auto alloc = allocations[target];
        heap.free(alloc.ptr);
        log_free(alloc.ptr);
        allocations.erase(allocations.begin() + target);
      }
      break;
    }
  }

  for (const auto &alloc : allocations) {
    heap.free(alloc.ptr);
    log_free(alloc.ptr);
  }

  std::cout << "\nStress test completed successfully\n";
}

int main() {
  std::cout << "=========================================\n";
  std::cout << "   Buddy Memory Allocator Test Suite  \n";
  std::cout << "=========================================\n";

  test_lists();

  size_t expected_n = 1;
  size_t block_size = Buddy_allocation::k_size;

  while (block_size >= Min_alloc && block_size > sizeof(Block)) {
    std::cout << "=== Test: OOM after " << expected_n << " allocations of "
              << (block_size - sizeof(Block)) << " bytes ===\n";
    Buddy_allocation heap;
    check_oom_after(heap, expected_n, block_size - sizeof(Block));

    block_size /= 2;
    expected_n *= 2;
  }

  stress_test();

  std::cout << "\n All tests passed successfully.\n";
  return 0;
}
