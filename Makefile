# Compilador y flags
CC = g++
CFLAGS = -Wall -Wextra -Iinclude -MMD

# Directorios
SRC_DIR = src
BUILD_DIR = build
INCLUDE_DIR = include
TARGET = $(BUILD_DIR)/megaNew

# Archivos fuente, objetos y dependencias
SRC = $(wildcard $(SRC_DIR)/*.cpp)
OBJ = $(SRC:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
DEPS = $(OBJ:.o=.d)

# Regla principal
all: $(TARGET)

# Enlazado del ejecutable
$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $@

# Compilación de cada archivo .cpp a .o, generando también archivos .d
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# Incluir dependencias de headers
-include $(DEPS)

# Limpieza total
clean:
	rm -rf build/*
.PHONY: all clean

