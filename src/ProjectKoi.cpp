#include <ProjectKoi.h>

// TODO:
//       Fix texture & model duplication
//       Move Keybinds to Input System? Map (key -> function -> immediate message)
//       Implement OVR Context & OVR Renderer 

ProjectKoi::ProjectKoi()
{

}

ProjectKoi::~ProjectKoi()
{

}

void ProjectKoi::init()
{
    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
    auto epoch = now_ms.time_since_epoch();
    auto value = std::chrono::duration_cast<std::chrono::milliseconds>(epoch);
    lastUpdateTime = value.count();

    setMessageCallback(Exit, (message_method_t) &ProjectKoi::exit);
}

void ProjectKoi::run()
{
    while (!this->needsDestroying)
    {
        auto now = std::chrono::system_clock::now();
        auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
        auto epoch = now_ms.time_since_epoch();
        auto value = std::chrono::duration_cast<std::chrono::milliseconds>(epoch);
        long elapsedTime = value.count() - lastUpdateTime;
        lastUpdateTime = value.count();

        this->update(elapsedTime);
        this->renderSystem->draw();
    }
}

void ProjectKoi::update(long elapsedTime)
{
	while (!(msgQueue.empty()))
    {
        Message * msg = msgQueue.front();
        this->handleMessage(msg);
        sendMessageNow(msg);
        msgQueue.pop();
    }

    for (auto& system : registeredSystems)
        system->update(elapsedTime);
}

void ProjectKoi::exit(Message * msg)
{
    this->needsDestroying = true;
}

#ifndef ANDROID

int main()
{
    ProjectKoi app;

    app.init();

    app.inputSystem = new InputSystem();
    app.registerSystem(app.inputSystem);

    app.renderSystem = new RenderSystem();
    app.registerSystem(app.renderSystem);

    app.run();

    delete app.renderSystem;
    delete app.inputSystem;

    return 0;
}

#else

#include <android_native_app_glue.h>

void android_main(android_app * android_context)
{
    ProjectKoi app;

    app.init();

    app.renderSystem = new RenderSystem(android_context);
    app.registerSystem(app.renderSystem);

    app.run();

    delete app.renderSystem;
}

#endif