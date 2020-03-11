
#include <RenderSystem.h>

#include <ios>
#include <cmath>

void RenderSystem::init()
{
	context = new DesktopContext();
	app->registerSystem(context);

	renderer = new DesktopRenderer(context);
	app->registerSystem(renderer);

	scene = new Scene3D(context, renderer);
	app->registerSystem(scene);

	gui = new GUI(context, renderer, app);

	//gui = new RenderFramework::GUI(context, renderer, parent);

	// Register Message Actions
	setMessageCallback(SetWindowFocus, (message_method_t) &RenderSystem::onWindowFocus);
	setMessageCallback(KeyPress, (message_method_t) &RenderSystem::onKeyPress);

	DEBUG("RENDER_SYSTEM - RenderSystem Created");
}

void RenderSystem::update(long elapsedTime)
{

}

void RenderSystem::draw()
{
	VkCommandBuffer drawBuffer = renderer->getNextCommandBuffer();
	scene->draw(drawBuffer);
	gui->draw(drawBuffer);
	renderer->render(drawBuffer);
	renderer->present();
}

RenderSystem::~RenderSystem()
{
	vkQueueWaitIdle(context->primaryGraphicsQueue->queue);
	
	delete gui;
	delete scene;
	delete renderer;
	delete context;

	DEBUG("RENDER_SYSTEM - RenderSystem Destroyed");
}

void RenderSystem::onKeyPress(Message * msg)
{

}

void RenderSystem::onWindowFocus(Message * msg)
{
	IntegerMessage * intmsg = dynamic_cast<IntegerMessage *> (msg);
	paused = intmsg->data == 0;
}