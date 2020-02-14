
SYSTEM_INCLUDE = -Isystem -Isystem/framework
CONSOLE_INCLUDE = -Iconsole
RENDER_INCLUDE = -Irenderer -Irenderer/framework
INPUT_INCLUDE = -Iinput
CAMERA_INCLUDE = -Icamera

VULKAN_INCLUDE = -I$(VULKAN_SDK)/include
EXTRA_INCLUDE = -I3rd_party

INCLUDES = -I. $(SYSTEM_INCLUDE) $(CONSOLE_INCLUDE) $(RENDER_INCLUDE) $(INPUT_INCLUDE) $(CAMERA_INCLUDE) $(VULKAN_INCLUDE) $(EXTRA_INCLUDE)

LINUX_CFLAGS = -std=c++17 $(INCLUDES) -DLINUX -DSUPPRESS_DEBUG
LINUX_LDFLAGS = -L$(VULKAN_SDK)/lib `pkg-config --static --libs glfw3` -lvulkan

SYSTEM_FRAMEWORK = system/MessageBus.cpp
CONSOLE = console/ConsoleSystem.cpp
RENDERER = renderer/RenderSystem.cpp renderer/framework/RenderFramework.cpp renderer/framework/Context.cpp renderer/framework/Renderer.cpp renderer/framework/Scene3D.cpp
INPUT = input/InputSystem.cpp
CAMERA = camera/CameraSystem.cpp

SYSTEMS = $(SYSTEM_FRAMEWORK) $(CONSOLE) $(RENDERER) $(INPUT) $(CAMERA)

ProjectKoi: ProjectKoi.cpp $(SYSTEMS)
	bash compile_shaders.sh
	g++ -g $(LINUX_CFLAGS) -o ProjectKoi ProjectKoi.cpp $(SYSTEMS) $(LINUX_LDFLAGS)
	./ProjectKoi
	rm -f ProjectKoi
