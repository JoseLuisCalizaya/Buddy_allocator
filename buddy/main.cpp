#include "buddy.h"
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

// La estructura VRAMResource
struct VRAMResource {
  void *ptr = nullptr;
  size_t requested_size = 0;
  size_t allocated_size = 0;
  std::string name;

  VRAMResource(std::string n, void *p, size_t req_s, size_t alloc_s)
      : ptr(p), requested_size(req_s), allocated_size(alloc_s),
        name(std::move(n)) {}
};

class VRAMManager {
private:
  Buddy_allocation heap;
  std::vector<VRAMResource> resources;
  size_t total_requested_memory = 0;
  size_t total_allocated_memory = 0;

  void log(const std::string &message) {
    std::cout << "[VRAM Manager] " << message << std::endl;
  }

public:
  VRAMManager() {
    log("Iniciando subsistema de memoria gráfica.");
    std::cout << "  > Tamaño total de VRAM simulada: "
              << (Buddy_allocation::k_size / 1024) << " KB\n";
    std::cout << "  > Asignación mínima (tamaño de metadatos): " << Min_alloc
              << " bytes\n"
              << std::endl;
  }

  void *allocate(const std::string &name, size_t size) {
    log("Solicitud de asignación para '" + name + "' de " +
        std::to_string(size) + " bytes.");
    void *ptr = heap.malloc(size);

    if (!ptr) {
      std::cerr << "  ERROR: ¡Fallo de asignación! (Out of Memory en VRAM)\n"
                << std::endl;
      return nullptr;
    }

    Block *block_header =
        reinterpret_cast<Block *>(static_cast<char *>(ptr) - sizeof(Block));
    size_t actual_size = block_header->allocate_size;

    resources.emplace_back(name, ptr, size, actual_size);

    total_requested_memory += size;
    total_allocated_memory += actual_size;

    std::cout << "  > Éxito. Asignado en la dirección " << ptr
              << ". Bloque real: " << actual_size << " bytes.\n"
              << std::endl;

    return ptr; // Devolvemos el puntero estable
  }

  void free(void *ptr) {
    if (!ptr)
      return;
    auto it =
        std::find_if(resources.begin(), resources.end(),
                     [ptr](const VRAMResource &res) { return res.ptr == ptr; });

    if (it != resources.end()) {
      log("Liberando recurso '" + it->name + "' (" +
          std::to_string(it->requested_size) + " bytes).");

      total_requested_memory -= it->requested_size;
      total_allocated_memory -= it->allocated_size;

      // Liberamos la memoria usando el Buddy Allocator
      heap.free(it->ptr);

      // Eliminamos el registro del vector
      resources.erase(it);

      std::cout << "  > Memoria en " << ptr << " liberada.\n" << std::endl;
    }
  }

  void print_report() {
    double fragmentation_percentage = 0.0;
    if (total_allocated_memory > 0) {
      fragmentation_percentage =
          static_cast<double>(total_allocated_memory - total_requested_memory) /
          total_allocated_memory * 100.0;
    }

    std::cout << "=========================================\n";
    std::cout << "         INFORME DE ESTADO DE VRAM        \n";
    std::cout << "-----------------------------------------\n";
    std::cout << "Recursos actualmente en memoria: " << resources.size()
              << "\n";
    std::cout << "Memoria solicitada (útil):    "
              << total_requested_memory / 1024.0 << " KB\n";
    std::cout << "Memoria asignada (real):      "
              << total_allocated_memory / 1024.0 << " KB\n";
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Fragmentación interna total:  "
              << (total_allocated_memory - total_requested_memory) / 1024.0
              << " KB (" << fragmentation_percentage << "%)\n";
    std::cout << "=========================================\n\n";
  }
};

void simulate_graphics_pipeline() {
  std::cout << "=====================================================\n";
  std::cout << "   Simulación de Pipeline Gráfico con Buddy Allocator \n";
  std::cout << "=====================================================\n\n";

  VRAMManager vram;

  // --- FASE 1: Las variables ahora guardan void*, que son estables ---
  std::cout << "--- FASE 1: Cargando recursos de la escena ---\n\n";
  void *main_shader = vram.allocate("Shader Principal", 25000);
  void *skybox_texture = vram.allocate("Textura Skybox", 500000);
  void *player_model = vram.allocate("Modelo 3D Jugador", 120000);
  vram.print_report();

  // --- FASE 2: ---
  std::cout << "--- FASE 2: Renderizando frames (creando recursos temporales) "
               "---\n\n";
  for (int frame = 1; frame <= 3; ++frame) {
    std::cout << "--- Renderizando Frame " << frame << " ---\n";
    void *post_processing_buffer = vram.allocate("Buffer Post-FX", 200000);
    vram.free(post_processing_buffer);
  }
  std::cout << "--- Fin del renderizado de frames ---\n\n";
  vram.print_report();

  // --- FASE 3: ---
  std::cout << "--- FASE 3: Cargando recurso dinámico ---\n\n";
  void *enemy_model = vram.allocate("Modelo 3D Enemigo", 90000);
  vram.print_report();

  // --- FASE 4: Ahora llamamos a free con los punteros estables ---
  std::cout
      << "--- FASE 4: Cerrando aplicación y liberando toda la VRAM ---\n\n";
  vram.free(main_shader);
  vram.free(skybox_texture);
  vram.free(player_model);
  vram.free(enemy_model);

  vram.print_report();
}

int main() {
  simulate_graphics_pipeline();
  return 0;
}
