#ifndef GUI_H
#define GUI_H

#include <vector>

#include <render/DesktopContext.h>
#include <render/DesktopRenderer.h>
#include <render/Scene3D.h>

#include <imgui/imgui.h>
#include <imgui/examples/imgui_impl_glfw.h>
#include <imgui/examples/imgui_impl_vulkan.h>

class GUIElement
{
    public:
    GUIElement();
    virtual ~GUIElement() = 0;

    virtual void draw() = 0;
};

class FPSMeter : public GUIElement, public System
{
    uint32_t frameCount;
    long timeBeforeFPSUpdate;
    uint32_t FPS;

    public:
    FPSMeter();
    ~FPSMeter();

    void init();
    void update(long elapsedTime);
    void draw();
};

class Console;
typedef void (Console::*command_method_t)(std::vector<std::string> args);

class Console : public GUIElement, public System
{
    char buffer[128];
    std::vector<std::string> history;
    uint32_t historyIndex;

    std::unordered_map<long, command_method_t> commands;

    void processCommand(std::string command);
    static std::vector<std::string> parseCommandArgs(std::string);
    static long hashCode(std::string);

    public:
    Console();
    ~Console();

    void init();
    void update(long elapsedTime);
    void draw();

    int onConsoleUpdate(ImGuiInputTextCallbackData * data);

    void exit(std::vector<std::string> args);
    void addModel(std::vector<std::string> args);
};

class LightingTweaker : public GUIElement, public System
{
    DirectionalLightData data;

    public:
    LightingTweaker();
    ~LightingTweaker();

    void init();
    void update(long elapsedTime);
    void draw();
};

class ModelViewer : public GUIElement, public System
{
    public:
    std::vector<ModelData> models;

    ModelViewer();
    ~ModelViewer();

    void init();
    void update(long elapsedTime);
    void draw();

    void getModels(Message * msg);
};

class GUI
{
    public:
    Context * context;
    Renderer * renderer;

    std::vector<GUIElement *> elements;

    VkDescriptorPool descriptorPool;
    VkRenderPass renderPass;
    std::vector<VkFramebuffer> framebuffers;

    GUI(DesktopContext * context, DesktopRenderer * renderer, MessageBus * app);
    ~GUI();

    void draw(VkCommandBuffer commandbuffer);
};

#endif