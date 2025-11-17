# --- Compilador y Flags ---
CXX = g++
# Añadimos -Ihead para que el compilador sepa dónde buscar los .h
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -MMD -MP -Ihead

# --- Nombres de Archivos y Directorios ---

# Nombre del ejecutable final
TARGET = test_buddy

# Directorio para los archivos .o y .d generados
OBJ_DIR = obj

# Directorio de los archivos .cpp (excepto main)
SRC_DIR = src

# Archivo .cpp principal (en el root)
MAIN_SRC = main.cpp

# Lista de los OTROS archivos .cpp en el directorio 'src'
# Si añades más (ej. slab.cpp), solo añádelos a esta lista
SRCS = block.cpp buddy.cpp list.cpp

# --- Generación Automática de Rutas ---
# (No necesitas tocar esta parte)

# Convierte la lista de .cpp en rutas a archivos .o en OBJ_DIR
# e.g., "block.cpp" -> "obj/block.o"
OBJS = $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(SRCS))

# Convierte el .cpp principal en su .o en OBJ_DIR
# e.g., "main.cpp" -> "obj/main.o"
MAIN_OBJ = $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(MAIN_SRC))

# Lista completa de todos los archivos .o
ALL_OBJS = $(MAIN_OBJ) $(OBJS)

# Lista de todos los archivos .d (dependencias)
DEPS = $(ALL_OBJS:.o=.d)

# --- Reglas de Compilación ---

# Regla 'all' (por defecto): crear el ejecutable
.PHONY: all clean
all: $(TARGET)

# Regla de enlace: crea el ejecutable final a partir de todos los .o
$(TARGET): $(ALL_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# --- Reglas de Compilación ---

# Regla para compilar main.cpp (fuente en el root, .o en obj/)
$(OBJ_DIR)/main.o: main.cpp
	@mkdir -p $(@D) # Crea el directorio 'obj' si no existe
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Regla patrón para compilar los .cpp de src/
# (fuente en 'src/', .o en 'obj/')
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(@D) # Crea el directorio 'obj' si no existe
	$(CXX) $(CXXFLAGS) -c $< -o $@

# --- Dependencias y Limpieza ---

# Incluye los archivos de dependencia generados
-include $(DEPS)

# Regla para limpiar el proyecto
clean:
	@echo "Limpiando proyecto..."
	@rm -f $(TARGET)
	@rm -rf $(OBJ_DIR) # Elimina todo el directorio 'obj'
