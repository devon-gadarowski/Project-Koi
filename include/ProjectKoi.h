#ifndef PROJECT_KOI_H
#define PROJECT_KOI_H

#include <chrono>

#include <system/Message.h>
#include <system/System.h>

#include <RenderSystem.h>
#include <InputSystem.h>

class ProjectKoi : public System, public MessageBus
{
    long lastUpdateTime;
    bool needsDestroying = false;

    public:
    ProjectKoi();
    ~ProjectKoi();

    RenderSystem * renderSystem;
    InputSystem * inputSystem;

    void init();
    void update(long elapsedTime);

    void run();
    void exit(Message * msg);
};

#endif