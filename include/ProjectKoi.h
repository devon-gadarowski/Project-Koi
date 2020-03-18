#ifndef PROJECT_KOI_H
#define PROJECT_KOI_H

#include <chrono>

#include <system/Message.h>
#include <system/System.h>

#include <RenderSystem.h>

#ifndef ANDROID

#include <InputSystem.h>

#else

#include <vulkan_wrapper.h>

#endif

class ProjectKoi : public System, public MessageBus
{
    long lastUpdateTime;
    bool needsDestroying = false;

    public:
    ProjectKoi();
    ~ProjectKoi();

    RenderSystem * renderSystem;

#ifndef ANDROID
    InputSystem * inputSystem;
#endif

    void init();
    void update(long elapsedTime);

    void run();
    void exit(Message * msg);
};

#endif