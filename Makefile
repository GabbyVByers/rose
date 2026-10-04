
# make clean - deletes the build directory
# make build - builds the program
# make run   - builds and runs the program
# make start - cleans, builds, and runs (full rebuild)
#
# Build requires the Vulkan SDK (https://vulkan.lunarg.com)

ifeq ($(VULKAN_SDK),)
    $(error Vulkan SDK is required: (see https://vulkan.lunarg.com))
endif

VULKAN_SDK_DIR := $(subst \,/,$(VULKAN_SDK))

CC       := gcc
CPPFLAGS := -DUNICODE -D_UNICODE -DWIN32_LEAN_AND_MEAN -isystem $(VULKAN_SDK_DIR)/Include
CFLAGS   := -std=c17 -Wall -Wextra -Wpedantic -g -O0
LDFLAGS  := -L$(VULKAN_SDK_DIR)/Lib
LDLIBS   := -lvulkan-1 -luser32 -lgdi32

SRC_DIR    := source
SHADER_DIR := shaders
BUILD_DIR  := build
TARGET     := app

ifeq ($(OS),Windows_NT)
    EXE := .exe
endif

SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/obj/%.o)
DEPS := $(OBJS:.o=.d)
BIN  := $(BUILD_DIR)/$(TARGET)$(EXE)

GLSLC       := $(VULKAN_SDK_DIR)/Bin/glslc
SHADER_OUTS := $(BUILD_DIR)/shaders/vertex.spv $(BUILD_DIR)/shaders/fragment.spv

.PHONY: all build run clean start

all: build

build: $(BIN) $(SHADER_OUTS)

$(BIN): $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) $(LDLIBS) -o $@

$(BUILD_DIR)/obj/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR)/shaders/vertex.spv: $(SHADER_DIR)/shader.vert
	@mkdir -p $(@D)
	$(GLSLC) $< -o $@

$(BUILD_DIR)/shaders/fragment.spv: $(SHADER_DIR)/shader.frag
	@mkdir -p $(@D)
	$(GLSLC) $< -o $@

run: build
	./$(BIN)

clean:
	rm -rf $(BUILD_DIR)

-include $(DEPS)

start:
	$(MAKE) clean
	$(MAKE) run

