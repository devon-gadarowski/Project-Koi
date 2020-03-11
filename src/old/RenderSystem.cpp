
#include <RenderSystem.h>

#include <ios>
#include <cmath>

void RenderSystem::update(long elapsedTime)
{
	glfwPollEvents();
	if (glfwWindowShouldClose(context->window))
	{
		parent->sendMessageNow(Shutdown);
	}

	if (paused)
		return;

	if (!sceneLoaded)
	{
		std::string defaultScene("porsche.json");
		parent->sendMessageNow(LoadScene, defaultScene);
		return;
	}

	uint32_t imageIndex = renderer->getNextImage();

	std::vector<VkCommandBuffer> commandBuffers;

	scene->updateUBO(imageIndex);
	gui->update(elapsedTime);
	
	commandBuffers.push_back(scene->getFrame(imageIndex));
	commandBuffers.push_back(gui->getFrame(imageIndex));

	renderer->render(imageIndex, commandBuffers);
	renderer->present(imageIndex);
}

RenderSystem::RenderSystem()
{
	DEBUG("RENDER_SYSTEM - RenderSystem Created");

	// Register Message Actions
	setMessageCallback(LoadScene, (message_method_t) &RenderSystem::loadScene);
	setMessageCallback(SetWindowFocus, (message_method_t) &RenderSystem::onWindowFocus);
	setMessageCallback(KeyPress, (message_method_t) &RenderSystem::onKeyPress);
	setMessageCallback(SetCameraPosition, (message_method_t) &RenderSystem::onCameraPositionUpdate);
	setMessageCallback(SetCameraDirection, (message_method_t) &RenderSystem::onCameraDirectionUpdate);
}

RenderSystem::~RenderSystem()
{
	shutdown();

	DEBUG("RENDER_SYSTEM - RenderSystem Destroyed");
}

void RenderSystem::init()
{
	if (initialized)
		return;

	context = new RenderFramework::Context();
	parent->sendMessageNow(GLFWwindowCreated, (void *) context->window);

	renderer = new RenderFramework::Renderer(context);
	gui = new RenderFramework::GUI(context, renderer, parent);

	initialized = true;
}

void RenderSystem::shutdown()
{
	paused = true;

	stopScene();

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

void RenderSystem::loadScene(Message * msg)
{
	StringMessage * strmsg = dynamic_cast<StringMessage *> (msg);

	try
	{
		std::string sceneName = strmsg->data;

		DEBUG("Scene name: %s", sceneName.c_str());

		RenderFramework::Scene3D * newScene = new RenderFramework::Scene3D(context, renderer, sceneName);

		if (sceneLoaded)
			stopScene();

		this->scene = newScene;
		sceneLoaded = true;

		parent->sendMessage(SceneLoaded, scene);
	}
	catch (int e)
	{
		WARN("RENDER_SYSTEM - Failed to load scence %s", strmsg->data.c_str());
	}
}

void RenderSystem::stopScene()
{
	if (sceneLoaded)
	{
		sceneLoaded = false;
		delete scene;
	}

	scene = nullptr;
	parent->sendMessage(SceneDestroyed);
}

void RenderSystem::onKeyPress(Message * msg)
{

}

void RenderSystem::onWindowFocus(Message * msg)
{
	IntegerMessage * intmsg = dynamic_cast<IntegerMessage *> (msg);
	paused = intmsg->data == 0;
}

void RenderSystem::onCameraPositionUpdate(Message * msg)
{
	if (!sceneLoaded)
		return;

	VectorMessage * position = dynamic_cast<VectorMessage *> (msg);

	scene->camera.position.x = position->data[0];
	scene->camera.position.y = position->data[1];
	scene->camera.position.z = position->data[2];
}

void RenderSystem::onCameraDirectionUpdate(Message * msg)
{
	if (!sceneLoaded)
		return;

	VectorMessage * direction = dynamic_cast<VectorMessage *> (msg);

	scene->camera.direction.x = direction->data[0];
	scene->camera.direction.y = direction->data[1];
	scene->camera.direction.z = direction->data[2];
}
