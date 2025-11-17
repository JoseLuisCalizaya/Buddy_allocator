#include "buddy.h"
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
// La estructura VRAMResource
struct VRAMResource {
  void *ptr = nullptr;
  size_t requested_size = 0;
  size_t allocated_size = 0;
  std::string name;
  int width = 0;
  int height = 0;
  int channels = 0;
  VRAMResource(std::string n, size_t req_s)
      : requested_size(req_s), name(std::move(n)) {}
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

    resources.emplace_back(name, size);
    VRAMResource &resource = resources.back();
    resource.ptr = ptr;
    resource.allocated_size = actual_size;

    total_requested_memory += size;
    total_allocated_memory += actual_size;

    std::cout << "  > Éxito. Asignado en la dirección " << ptr
              << ". Bloque real: " << actual_size << " bytes.\n"
              << std::endl;

    return &resource;
  }

  void free(void *ptr) {
    if (!ptr)
      return;
    // Buscamos el recurso en nuestro vector usando el puntero 'ptr' como
    // identificador único.
    auto it =
        std::find_if(resources.begin(), resources.end(),
                     [ptr](const VRAMResource &res) { return res.ptr == ptr; });

    if (it != resources.end()) {
      log("Liberando recurso '" + it->name + "' (" +
          std::to_string(it->requested_size) + " bytes).");

      // Actualizamos nuestras estadísticas
      total_requested_memory -= it->requested_size;
      total_allocated_memory -= it->allocated_size;

      // Liberamos la memoria real usando el Buddy Allocator
      heap.free(it->ptr);

      // Eliminamos el registro del recurso de nuestro vector
      resources.erase(it);

      std::cout << "> Memoria en " << ptr << " liberada.\n" << std::endl;
    } else {
      // Opcional: un mensaje por si se intenta liberar un puntero desconocido
      std::cerr << "  ADVERTENCIA: Se intentó liberar un puntero de memoria no "
                   "reconocido: "
                << ptr << std::endl;
    }
  }

  VRAMResource *load_image_to_vram(const std::string &filename) {
    log("Iniciando carga de imagen: " + filename);
    int width, height, channels;

    // Primero, usamos stbi para obtener las dimensiones y calcular el tamaño
    // necesario
    unsigned char *temp_data =
        stbi_load(filename.c_str(), &width, &height, &channels, 0);
    if (!temp_data) {
      std::cerr << "  ERROR: No se pudo cargar la imagen '" << filename
                << "'.\n"
                << std::endl;
      return nullptr;
    }
    size_t image_size = width * height * channels;
    stbi_image_free(temp_data); // Liberamos la memoria temporal

    // Ahora, solicitamos memoria a nuestro Buddy Allocator
    VRAMResource *image_resource =
        (VRAMResource *)allocate(filename, image_size);
    if (!image_resource) {
      return nullptr; // No hay suficiente memoria en nuestra VRAM
    }

    // Finalmente, cargamos los datos de la imagen DIRECTAMENTE en el bloque de
    // memoria asignado
    stbi_load(filename.c_str(), &width, &height, &channels, 0);
    unsigned char *image_data_in_vram =
        static_cast<unsigned char *>(image_resource->ptr);
    stbi_load_from_memory(image_data_in_vram, image_resource->allocated_size,
                          &width, &height, &channels, 0);

    image_resource->width = width;
    image_resource->height = height;
    image_resource->channels = channels;

    std::cout << "  > Imagen '" << filename << "' (" << width << "x" << height
              << ", " << channels << " canales)";
    std::cout << " cargada exitosamente en la VRAM.\n" << std::endl;

    return image_resource;
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

void simulate_real_resource_loading() {
  std::cout << "=====================================================\n";
  std::cout << "   Simulación con Carga de Recursos Gráficos Reales \n";
  std::cout << "=====================================================\n\n";

  VRAMManager vram;
  printf("Inicio\n");
  vram.print_report();

  // --- FASE 1: Cargar una imagen real desde el disco ---
  // Asegúrate de tener una imagen llamada "test_image.png" en el mismo
  // directorio
  auto texture1 = vram.load_image_to_vram("prueba01.jpg");
  if (!texture1) {
    std::cerr << "Finalizando simulación debido a error de carga.\n";
    return;
  }
  vram.print_report();

  // --- FASE 2: Cargar un recurso genérico como un shader ---
  auto main_shader = vram.allocate("Shader de Iluminación", 35000);
  printf("Carga del main_shader");
  vram.print_report();

  // --- FASE 3: Liberar la imagen para hacer espacio ---
  // vram.free(texture1);
  // vram.print_report();

  // --- FASE 4: Cargar otra imagen (si tienes otra) ---
  auto texture2 = vram.load_image_to_vram("prueba02.jpg");
  if (!texture2) {
    std::cerr << "Error de carga de textura 02\n";
    return;
  }
  vram.print_report();

  // --- FASE 5: Liberación final ---
  vram.free(main_shader);
  vram.free(texture1);
  vram.free(texture2);
  printf("Liberacion\n");
  vram.print_report();
}
int main() {
  // simulate_graphics_pipeline();
  simulate_real_resource_loading();
  return 0;
}
