#include <ProjectKoi.h>

// TODO: Solidify System constructor/initializer relationship
//       Add Immediate Mode GUI elements
//       Optimize Event System to improve tick rate
//       Move Keybinds to Input System? Map (key -> function -> immediate message)
//       Implement OVR Context & OVR Renderer 

ProjectKoi::ProjectKoi()
{

}

ProjectKoi::~ProjectKoi()
{
    delete renderSystem;
    delete inputSystem;
}

void ProjectKoi::init()
{
    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
    auto epoch = now_ms.time_since_epoch();
    auto value = std::chrono::duration_cast<std::chrono::milliseconds>(epoch);
    lastUpdateTime = value.count();

    setMessageCallback(Exit, (message_method_t) &ProjectKoi::exit);

    inputSystem = new InputSystem();
    this->registerSystem(inputSystem);

    renderSystem = new RenderSystem();
    this->registerSystem(renderSystem);
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

int main()
{
    ProjectKoi app;

    app.init();
    app.run();

    return 0;
}