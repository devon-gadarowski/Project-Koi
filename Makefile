CXX = g++
CXX_FLAGS = -std=c++17 -ggdb

BIN = bin
SRC = src
INCLUDE = include

LIBRARIES = -L$(VULKAN_SDK)/lib  -lvulkan `pkg-config --static --libs glfw3`
EXECUTABLE = ProjectKoi

3RD_PARTY = 3rd_party

VULKAN_INCLUDE = $(VULKAN_SDK)/include

SHADERS = assets/shaders

all: clean $(BIN)/$(EXECUTABLE) build_shaders

run: all
	clear
	./$(BIN)/$(EXECUTABLE)

$(BIN)/$(EXECUTABLE): $(SRC)/*.cpp $(3RD_PARTY)/dds/*.c $(3RD_PARTY)/imgui/*.cpp
	$(CXX) $(CXX_FLAGS) -I$(INCLUDE) -I$(3RD_PARTY) -I$(VULKAN_INCLUDE) $^ -o $@ $(LIBRARIES)

clean:
	rm -f $(BIN)/$(EXECUTABLE)
	rm -f $(BIN)/vert.spv
	rm -f $(BIN)/frag.spv

build_shaders:
	$(VULKAN_SDK)/bin/glslc $(SHADERS)/shader.vert -o $(BIN)/vert.spv
	$(VULKAN_SDK)/bin/glslc $(SHADERS)/shader.frag -o $(BIN)/frag.spv
