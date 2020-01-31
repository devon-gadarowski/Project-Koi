
#include <ProjectKoi.h>

#include <string.h>

#ifndef ANDROID
int main()
{
	MessageBus * msgBus = new MessageBus();

	msgBus->registerSystem(new ConsoleSystem());
	msgBus->registerSystem(new RenderSystem());
	msgBus->registerSystem(new InputSystem());
	msgBus->registerSystem(new CameraSystem());

	while (!msgBus->needsDestroying)
	{
		msgBus->update();
	}

	delete msgBus;

	return 0;
}
#else
void android_main(struct android_app * pApp)
{
	MessageBus * msgBus = new MessageBus();

	msgBus->registerSystem(new RenderSystem());

	while (!pApp->destroyRequested)
	{
		msgBus->update();
	}

	delete msgBus;

	return;
}
#endif
