# Compilador y flags
CC = gcc
CFLAGS = -Wall -Wextra -Iinclude -MMD

# Directorios
SRC_DIR = src
BUILD_DIR = build
INCLUDE_DIR = include
TARGET = $(BUILD_DIR)/mega

# Archivos fuente, objetos y dependencias
SRC = $(wildcard $(SRC_DIR)/*.c)
OBJ = $(SRC:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
DEPS = $(OBJ:.o=.d)

# Regla principal
all: $(TARGET)

# Enlazado del ejecutable
$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $@

# Compilación de cada archivo .c a .o, generando también archivos .d
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Incluir dependencias de headers
-include $(DEPS)

# Limpieza total
clean:
	@if exist build (del /s /q build\*.* > nul 2>&1)

.PHONY: all clean

