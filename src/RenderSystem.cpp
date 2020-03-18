
#include <RenderSystem.h>

#include <ios>
#include <cmath>

#ifndef ANDROID

void RenderSystem::init()
{
	context = new DesktopContext();
	app->registerSystem(context);

	renderer = new DesktopRenderer((dynamic_cast<DesktopContext *> (context)));
	app->registerSystem(renderer);

	scene = new Scene3D(context, renderer);
	app->registerSystem(scene);

	gui = new GUI(dynamic_cast<DesktopContext *> (context), dynamic_cast<DesktopRenderer *> (renderer), app);

	// Register Message Actions
	setMessageCallback(SetWindowFocus, (message_method_t) &RenderSystem::onWindowFocus);
	setMessageCallback(KeyPress, (message_method_t) &RenderSystem::onKeyPress);

	DEBUG("RENDER_SYSTEM - RenderSystem Created");
}

#else

void RenderSystem::init()
{
	context = new OVRContext(this->android_context);
	app->registerSystem(context);

	renderer = new OVRRenderer((dynamic_cast<OVRContext *> (context)));
	app->registerSystem(renderer);

	scene = new Scene3D(context, renderer);
	app->registerSystem(scene);

	DEBUG("RENDER_SYSTEM - RenderSystem Created");
}

#endif

void RenderSystem::update(long elapsedTime)
{

}

void RenderSystem::draw()
{
	VkCommandBuffer drawBuffer = renderer->getNextCommandBuffer();
	scene->draw(drawBuffer);

#ifndef ANDROID
	gui->draw(drawBuffer);
#endif

	renderer->render(drawBuffer);
	renderer->present();
}

RenderSystem::~RenderSystem()
{
	vkQueueWaitIdle(context->primaryGraphicsQueue->queue);
#ifndef ANDROID
	if (this->gui != nullptr) delete this->gui;
#endif
	if (this->scene != nullptr) delete scene;
	if (this->renderer != nullptr) delete renderer;
	if (this->context != nullptr) delete context;

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