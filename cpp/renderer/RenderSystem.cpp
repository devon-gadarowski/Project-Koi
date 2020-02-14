
#include <RenderSystem.h>

#include <ios>

void RenderSystem::update(long elapsedTime)
{
	if (!initialized)
	{
		return;
	}

	glfwPollEvents();

	if (glfwWindowShouldClose(context->window))
	{
		msgBus->sendMessageNow(Message(Shutdown));
	}

	if (!sceneLoaded || paused)
	{
		return;
	}

	scene->updateUBO();
	scene->draw();
}

RenderSystem::RenderSystem()
{
	DEBUG("RENDER_SYSTEM - RenderSystem Created");

	// Register Message Actions
	setMessageCallback(Initialize, (message_method_t) &RenderSystem::initialize);
	setMessageCallback(Shutdown, (message_method_t) &RenderSystem::shutdown);
	setMessageCallback(LoadScene, (message_method_t) &RenderSystem::loadScene);
	setMessageCallback(SetWindowFocus, (message_method_t) &RenderSystem::onWindowFocus);
	setMessageCallback(SetCameraPosition, (message_method_t) &RenderSystem::onCameraPositionUpdate);
	setMessageCallback(SetCameraDirection, (message_method_t) &RenderSystem::onCameraDirectionUpdate);
}

RenderSystem::~RenderSystem()
{
	if (scene != nullptr)
		delete scene;

	if (renderer != nullptr)
		delete renderer;

	if (context != nullptr)
		delete context;

	DEBUG("RENDER_SYSTEM - RenderSystem Destroyed");
}

void RenderSystem::initialize(void * data)
{
	if (initialized)
		return;

	context = new RenderFramework::Context();
	renderer = new RenderFramework::Renderer(context);

	msgBus->sendMessage(Message(GLFWwindowCreated, context->window));

	initialized = true;
}

void RenderSystem::shutdown(void * data)
{
	if (sceneLoaded)
		stopScene(nullptr);

	if (renderer != nullptr)
		delete renderer;

	if (context != nullptr)
		delete context;

	scene = nullptr;
	renderer = nullptr;
	context = nullptr;

	initialized = false;
	sceneLoaded = false;
}

void RenderSystem::loadScene(void * data)
{
	if (sceneLoaded)
		stopScene(nullptr);

	if (!initialized)
		initialize(nullptr);

	try
	{
		scene = new RenderFramework::Scene3D(context, renderer, *((std::string *) data));
		sceneLoaded = true;

		msgBus->sendMessage(Message(SceneLoaded, scene));
	}
	catch (const std::ios_base::failure& e)
	{
		WARN("RENDER_SYSTEM - Failed to load scence %s", ((std::string *) data)->c_str());
		shutdown(nullptr);
	}
}

void RenderSystem::stopScene(void * data)
{
	if (scene != nullptr)
		delete scene;

	scene = nullptr;
	sceneLoaded = false;

	msgBus->sendMessage(Message(SceneDestroyed, nullptr));
}

void RenderSystem::onWindowFocus(void * data)
{
	paused = reinterpret_cast<long> (data) == 0;
}

void RenderSystem::onCameraPositionUpdate(void * data)
{
	if (!sceneLoaded)
		return;

	float * position = (float *) data;

	scene->camera.center.x -= scene->camera.position.x;
	scene->camera.center.y -= scene->camera.position.y;
	scene->camera.center.z -= scene->camera.position.z;

	scene->camera.center.x += position[0];
	scene->camera.center.y += position[1];
	scene->camera.center.z += position[2];

	scene->camera.position.x = position[0];
	scene->camera.position.y = position[1];
	scene->camera.position.z = position[2];
}

void RenderSystem::onCameraDirectionUpdate(void * data)
{
	if (!sceneLoaded)
		return;

	float * direction = (float *) data;

	scene->camera.center.x = scene->camera.position.x + direction[0];
	scene->camera.center.y = scene->camera.position.y + direction[1];
	scene->camera.center.z = scene->camera.position.z + direction[2];
}
