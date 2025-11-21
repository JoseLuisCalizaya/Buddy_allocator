#include "head/buddy.h"
#include "head/slab.h"
#include <algorithm>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "head/stb_image.h" //
struct VRAMResource {
  void *ptr = nullptr;
  size_t requested_size = 0;
  size_t allocated_size = 0;
  std::string name;
  std::string allocate_type;
  VRAMResource(std::string n, void *p, size_t req_s, size_t alloc_s,
               std::string type)
      : ptr(p), requested_size(req_s), allocated_size(alloc_s),
        name(std::move(n)), allocate_type(type) {}
};

class VRAMManager {
private:
  Buddy_allocation buddy;
  SlabAllocator slab;
  std::vector<VRAMResource> resources;
  size_t total_requested_memory = 0;
  size_t total_allocated_memory = 0;

  void log(const std::string &message) {
    std::cout << "[VRAM Manager] " << message << std::endl;
  }

public:
  VRAMManager() { log("Iniciando Sistema Híbrido (Buddy + Slab)\n"); }

  ~VRAMManager() {
    log("Apagando el subsistema de memoria.");
    if (!resources.empty()) {
      std::cerr << "  ADVERTENCIA: ¡Quedaron " << resources.size()
                << " recursos sin liberar en la VRAM!\n";
    }
  }

  void *allocate(const std::string &name, size_t size) {
    // log("Solicitud de asignación para '" + name + "' de "
    // +std::to_string(size) + " bytes.");
    void *ptr = nullptr;
    size_t actual_size = 0;
    std::string type;

    if (size <= 256) {
      ptr = slab.allocate(size);
      if (ptr) {
        // Slab asigna tamaño fijo (32, 64, 128, 256)
        if (size <= 32)
          actual_size = 32;
        else if (size <= 64)
          actual_size = 64;
        else if (size <= 128)
          actual_size = 128;
        else
          actual_size = 256;
        type = "SLAB";
      }
    }
    if (!ptr) {
      ptr = buddy.malloc(size);
      if (ptr) {
        size_t power_of_2 = 16; // Min_alloc
        while (power_of_2 < size)
          power_of_2 *= 2;
        actual_size = power_of_2;
        type = "BUDDY";
      }
    }
    if (!ptr) {
      std::cerr << "Error: Out of Memory para " << name << "\n";
      return nullptr;
    }
    resources.emplace_back(name, ptr, size, actual_size, type);
    total_requested_memory += size;
    total_allocated_memory += actual_size;

    // std::cout << "Asignado: " << name << " (" << size << " bytes) -> " <<
    // type
    //           << " (" << actual_size << " bytes)\n";
    return ptr;
  }

  void *load_image_to_vram(const std::string &filename) {
    log("Iniciando carga de imagen: " + filename);
    int width, height, channels;

    unsigned char *data =
        stbi_load(filename.c_str(), &width, &height, &channels, 0); //
    if (!data) {
      std::cerr << "  Error al cargar la imagen" << filename << '\n';
      return nullptr;
    }
    size_t size = width * height * channels;
    void *ptr = allocate(filename, size);
    if (ptr) {
      memcpy(ptr, data, size);
      return nullptr;
    }

    stbi_image_free(data);
    return ptr;
  }

  void free(void *ptr) {
    if (!ptr)
      return;

    auto it =
        std::find_if(resources.begin(), resources.end(),
                     [ptr](const VRAMResource &r) { return r.ptr == ptr; });

    if (it != resources.end()) {
      // log("Liberando recurso '" + it->name + "'...");
      total_requested_memory -= it->requested_size;
      total_allocated_memory -= it->allocated_size;
      if (it->allocate_type == "SLAB")
        slab.free(ptr);
      else
        buddy.free(ptr);
      std::cout << "  > Memoria en " << ptr << " liberada.\n";
      resources.erase(it);
    } else {
      std::cerr << "  ADVERTENCIA: Se intentó liberar un puntero de memoria no "
                   "reconocido: "
                << ptr << '\n';
    }
  }
  void print_report() {
    double frag = 0;
    if (total_allocated_memory > 0)
      frag = 100.0 * (total_allocated_memory - total_requested_memory) /
             total_allocated_memory;

    std::cout << "\n=== REPORTE DE VRAM ===\n";
    std::cout << "Objetos en memoria: " << resources.size() << "\n";
    std::cout << "Memoria Solicitada: " << total_requested_memory << " bytes\n";
    std::cout << "Memoria Asignada:   " << total_allocated_memory << " bytes\n";
    std::cout << "Fragmentación:      " << std::fixed << std::setprecision(2)
              << frag << "%\n";
    std::cout << "=======================\n\n";
  }
};

// --- FUNCIÓN DE SIMULACIÓN CORREGIDA ---
void simulate_real_resource_loading() {
  std::cout << "=====================================================\n";
  std::cout << "   Simulación con Carga de Recursos Gráficos Reales \n";
  std::cout << "=====================================================\n\n";

  VRAMManager vram;
  printf("Inicio\n");
  vram.print_report();

  void *texture1 = vram.load_image_to_vram("prueba01.jpg");
  if (!texture1) {
    std::cerr << "Finalizando simulación debido a error de carga.\n";
    return;
  }
  vram.print_report();

  void *main_shader = vram.allocate("Shader de Iluminación", 35000);
  printf("Carga del main_shader\n");
  vram.print_report();

  void *texture2 = vram.load_image_to_vram("prueba02.jpg");
  if (!texture2) {
    std::cerr << "Error de carga de textura 02\n";
    return;
  }
  vram.print_report();

  printf("Liberacion\n");
  vram.free(main_shader);
  vram.free(texture1);
  vram.free(texture2);
  vram.print_report();
}

int main() {
  simulate_real_resource_loading();
  return 0;
}
