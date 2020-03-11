
#include <ProjectKoi.h>

int main()
{
	MessageBus * msgBus = new MessageBus();

	msgBus->registerSystem(new InputSystem());
	msgBus->registerSystem(new ConsoleSystem());
	msgBus->registerSystem(new RenderSystem());
	msgBus->registerSystem(new CameraSystem());

	while (!msgBus->needsDestroying)
	{
		msgBus->update();
	}

	delete msgBus;

	return 0;
}
