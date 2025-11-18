#include "buddy.h" //
#include <algorithm>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h" //
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
  Buddy_allocation heap; //
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
              << (Buddy_allocation::k_size / 1024) << " KB\n"; //
    std::cout << "  > Asignación mínima: " << Min_alloc << " bytes\n"
              << std::endl; //
  }

  ~VRAMManager() {
    log("Apagando el subsistema de memoria.");
    if (!resources.empty()) {
      std::cerr << "  ADVERTENCIA: ¡Quedaron " << resources.size()
                << " recursos sin liberar en la VRAM!\n";
    }
  }

  void *allocate(const std::string &name, size_t size) {
    log("Solicitud de asignación para '" + name + "' de " +
        std::to_string(size) + " bytes.");
    void *ptr = heap.malloc(size); //

    if (!ptr) {
      std::cerr << "  ERROR: ¡Fallo de asignación! (Out of Memory en VRAM)\n"
                << std::endl;
      return nullptr;
    }

    Block *block_header =
        reinterpret_cast<Block *>(static_cast<char *>(ptr) - sizeof(Block)); //
    size_t actual_size = block_header->allocate_size;

    resources.emplace_back(name, ptr, size, actual_size);
    total_requested_memory += size;
    total_allocated_memory += actual_size;

    std::cout << "  > Éxito. Asignado en la dirección " << ptr
              << ". Bloque real: " << actual_size << " bytes.\n"
              << std::endl;

    return ptr;
  }

  void *load_image_to_vram(const std::string &filename) {
    log("Iniciando carga de imagen: " + filename);
    int width, height, channels;

    unsigned char *temp_data =
        stbi_load(filename.c_str(), &width, &height, &channels, 0); //
    if (!temp_data) {
      std::cerr << "  ERROR: No se pudo cargar la imagen o no se encontró el "
                   "archivo '"
                << filename << "'.\n"
                << std::endl;
      return nullptr;
    }
    size_t image_size = width * height * channels;

    void *vram_ptr = allocate(filename, image_size);
    if (!vram_ptr) {
      stbi_image_free(temp_data);
      return nullptr;
    }

    memcpy(vram_ptr, temp_data, image_size);

    stbi_image_free(temp_data);

    std::cout << "  > Imagen '" << filename << "' (" << width << "x" << height
              << ", " << channels << " canales)";
    std::cout << " cargada exitosamente en la VRAM.\n" << std::endl;

    return vram_ptr;
  }

  void free(void *ptr) {
    if (!ptr)
      return;

    auto it =
        std::find_if(resources.begin(), resources.end(),
                     [ptr](const VRAMResource &res) { return res.ptr == ptr; });

    if (it != resources.end()) {
      log("Liberando recurso '" + it->name + "'...");
      total_requested_memory -= it->requested_size;
      total_allocated_memory -= it->allocated_size;
      heap.free(it->ptr); //
      resources.erase(it);
      std::cout << "  > Memoria en " << ptr << " liberada.\n" << std::endl;
    } else {
      std::cerr << "  ADVERTENCIA: Se intentó liberar un puntero de memoria no "
                   "reconocido: "
                << ptr << std::endl;
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
    std::cout << "  Recursos en memoria: " << resources.size() << "\n";
    std::cout << "  Memoria útil solicitada: "
              << total_requested_memory / 1024.0 << " KB\n";
    std::cout << "  Memoria real asignada:   "
              << total_allocated_memory / 1024.0 << " KB\n";
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "  Fragmentación interna: "
              << (total_allocated_memory - total_requested_memory) / 1024.0
              << " KB (" << fragmentation_percentage << "%)\n";
    std::cout << "=========================================\n\n";
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
