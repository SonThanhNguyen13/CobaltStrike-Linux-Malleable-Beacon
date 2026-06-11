# Makefile for Cobalt Strike Linux Implant

OUTPUT ?= beacon

CC = gcc
CFLAGS = -Wall -Wextra -O2 \
         -I./headers \
         -I./elfloader/elfloader-headers \
         $(EXTRA_CFLAGS)

LDFLAGS = -lssl -lcrypto -lcurl

# Directories
SRC_DIR            = src
ELFLOADER_SRC_DIR  = elfloader/elfloader-src
INC_DIR            = headers
ELFLOADER_INC_DIR  = elfloader/elfloader-headers
OBJ_DIR            = obj
BIN_DIR            = bin

# Target binary
TARGET = $(BIN_DIR)/$(OUTPUT)

# Source files
SOURCES = \
    $(SRC_DIR)/beacon.c \
    $(SRC_DIR)/commands.c \
    $(SRC_DIR)/crypto.c \
    $(SRC_DIR)/files.c \
    $(SRC_DIR)/http.c \
    $(SRC_DIR)/main.c \
    $(SRC_DIR)/pivot.c \
    $(SRC_DIR)/profile.c \
    $(SRC_DIR)/transform.c \
    $(SRC_DIR)/url_builder.c \
    $(wildcard $(ELFLOADER_SRC_DIR)/*.c)

# Object files (mirror source tree in obj/)
OBJECTS = $(patsubst %.c,$(OBJ_DIR)/%.o,$(SOURCES))

# Profile generated header - use .default if not generated
PROFILE_HEADER = $(INC_DIR)/profile_generated.h
PROFILE_DEFAULT = $(INC_DIR)/profile_generated.h.default

# Default target
all: check_profile directories $(TARGET)

check_profile:
	@if [ ! -f $(PROFILE_HEADER) ]; then \
		echo "[PROFILE] Using default profile (no generated profile found)"; \
		cp $(PROFILE_DEFAULT) $(PROFILE_HEADER); \
	fi

# Create necessary directories
directories:
	@mkdir -p $(BIN_DIR)
	@mkdir -p $(OBJ_DIR)/$(SRC_DIR)
	@mkdir -p $(OBJ_DIR)/$(ELFLOADER_SRC_DIR)

# Link the final binary
$(TARGET): $(OBJECTS)
	@echo "[LD] Linking $@"
	@$(CC) $(OBJECTS) -o $@ $(LDFLAGS)
	@echo "[+] Build complete: $@"

# Compile source files (works for all source dirs)
$(OBJ_DIR)/%.o: %.c
	@echo "[CC] Compiling $<"
	@$(CC) $(CFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	@echo "[CLEAN] Removing build artifacts"
	@rm -rf $(OBJ_DIR) $(BIN_DIR)
	@if [ -f $(PROFILE_HEADER) ] && [ $(PROFILE_HEADER) -ef $(PROFILE_DEFAULT) ]; then \
		rm -f $(PROFILE_HEADER); \
	fi

# Debug build
debug: CFLAGS += -g -DDEBUG
debug: clean all

.PHONY: all clean debug directories check_profile
