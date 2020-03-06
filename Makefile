CXX = g++
CXX_FLAGS = -std=c++17 -ggdb

BIN = bin
SRC = src
INCLUDE = include

LIBRARIES = -L$(VULKAN_SDK)/lib `pkg-config --static --libs glfw3` -lvulkan
EXECUTABLE = ProjectKoi

3RD_PARTY_SRC = $(SRC)/3rd_party
3RD_PARTY_INCLUDE = $(INCLUDE)/3rd_party

VULKAN_INCLUDE = $(VULKAN_SDK)/include

SHADERS = assets/shaders

all: clean $(BIN)/$(EXECUTABLE) build_shaders

run: all
	clear
	./$(BIN)/$(EXECUTABLE)

$(BIN)/$(EXECUTABLE): $(SRC)/*.cpp $(3RD_PARTY_SRC)/*.cpp $(3RD_PARTY_SRC)/*.c
	$(CXX) $(CXX_FLAGS) -I$(INCLUDE) -I$(3RD_PARTY_INCLUDE) -I$(VULKAN_INCLUDE) $^ -o $@ $(LIBRARIES)

clean:
	rm -f $(BIN)/$(EXECUTABLE)
	rm -f $(BIN)/vert.spv
	rm -f $(BIN)/frag.spv

build_shaders:
	$(VULKAN_SDK)/bin/glslc $(SHADERS)/shader.vert -o $(BIN)/vert.spv
	$(VULKAN_SDK)/bin/glslc $(SHADERS)/shader.frag -o $(BIN)/frag.spv
