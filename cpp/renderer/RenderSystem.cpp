
#include <RenderSystem.h>

#include <ios>
#include <cmath>

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

	uint32_t imageIndex = renderer->getNextImage();

	std::vector<VkCommandBuffer> commandBuffers(2);

	scene->updateUBO(imageIndex);
	commandBuffers[0] = scene->getFrame(imageIndex);

	gui->update(elapsedTime);
	commandBuffers[1] = gui->getFrame(imageIndex);

	renderer->render(imageIndex, commandBuffers);
	renderer->present(imageIndex);
}

RenderSystem::RenderSystem()
{
	DEBUG("RENDER_SYSTEM - RenderSystem Created");

	// Register Message Actions
	setMessageCallback(Initialize, (message_method_t) &RenderSystem::initialize);
	setMessageCallback(Shutdown, (message_method_t) &RenderSystem::shutdown);
	setMessageCallback(LoadScene, (message_method_t) &RenderSystem::loadScene);
	setMessageCallback(SetWindowFocus, (message_method_t) &RenderSystem::onWindowFocus);
	setMessageCallback(KeyPress, (message_method_t) &RenderSystem::onKeyPress);
	setMessageCallback(SetCameraPosition, (message_method_t) &RenderSystem::onCameraPositionUpdate);
	setMessageCallback(SetCameraDirection, (message_method_t) &RenderSystem::onCameraDirectionUpdate);
}

RenderSystem::~RenderSystem()
{
	shutdown(nullptr);

	DEBUG("RENDER_SYSTEM - RenderSystem Destroyed");
}

void RenderSystem::initialize(void * data)
{
	if (initialized)
		return;

	context = new RenderFramework::Context();
	renderer = new RenderFramework::Renderer(context);

	gui = new RenderFramework::GUI(context, renderer);

	msgBus->sendMessage(Message(GLFWwindowCreated, context->window));

	initialized = true;
}

void RenderSystem::shutdown(void * data)
{

	stopScene(nullptr);

	if (initialized)
	{
		initialized = false;
		delete gui;
		delete renderer;
		delete context;
	}

	scene = nullptr;
	gui = nullptr;
	renderer = nullptr;
	context = nullptr;
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
	catch (int e)
	{
		WARN("RENDER_SYSTEM - Failed to load scence %s", ((std::string *) data)->c_str());
		shutdown(nullptr);
	}
}

void RenderSystem::stopScene(void * data)
{
	if (sceneLoaded)
	{
		sceneLoaded = false;
		delete scene;
	}

	scene = nullptr;
	msgBus->sendMessage(Message(SceneDestroyed, nullptr));
}

void RenderSystem::onKeyPress(void * data)
{

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

	scene->camera.position.x = position[0];
	scene->camera.position.y = position[1];
	scene->camera.position.z = position[2];
}

void RenderSystem::onCameraDirectionUpdate(void * data)
{
	if (!sceneLoaded)
		return;

	float * direction = (float *) data;

	scene->camera.direction.x = direction[0];
	scene->camera.direction.y = direction[1];
	scene->camera.direction.z = direction[2];
}
