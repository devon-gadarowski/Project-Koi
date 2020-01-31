
#include <CameraSystem.h>

#include <cmath>

void CameraSystem::update(long elapsedTime)
{
	if (!running)
		return;

	bool updated = false;
	if (forward)
	{
		camera.setPosition(
			camera.position[0] + (camera.direction[0] * (speed * (float) elapsedTime/1000.0f)),
			camera.position[1] + (camera.direction[1] * (speed * (float) elapsedTime/1000.0f)),
			camera.position[2] + (camera.direction[2] * (speed * (float) elapsedTime/1000.0f))
		);
		updated = true;
	}

	if (backward)
	{
		camera.setPosition(
			camera.position[0] + (camera.direction[0] * (-speed * (float) elapsedTime/1000.0f)),
			camera.position[1] + (camera.direction[1] * (-speed * (float) elapsedTime/1000.0f)),
			camera.position[2] + (camera.direction[2] * (-speed * (float) elapsedTime/1000.0f))
		);
		updated = true;
	}

	if (up)
	{
		camera.setPosition(
			camera.position[0],
			camera.position[1] + (speed * (float) elapsedTime/1000.0f),
			camera.position[2]
		);
		updated = true;
	}

	if (down)
	{
		camera.setPosition(
			camera.position[0],
			camera.position[1] + (-speed * (float) elapsedTime/1000.0f),
			camera.position[2]
		);
		updated = true;
	}

	if (left)
	{
		camera.setPosition(
			camera.position[0] + (camera.direction[2] * (speed * (float) elapsedTime/1000.0f)),
			camera.position[1] + (camera.direction[1] * (speed * (float) elapsedTime/1000.0f)),
			camera.position[2] + (camera.direction[0] * (speed * (float) elapsedTime/1000.0f))
		);
		updated = true;
	}

	if (right)
	{
		camera.setPosition(
			camera.position[0] + (camera.direction[2] * (-speed * (float) elapsedTime/1000.0f)),
			camera.position[1] + (camera.direction[1] * (-speed * (float) elapsedTime/1000.0f)),
			camera.position[2] + (camera.direction[0] * (-speed * (float) elapsedTime/1000.0f))
		);
		updated = true;
	}

	if (mouseDelta[0] > 0)
	{
		//camera.direction[0] += mouseDelta[0] * (speed * (float) elapsedTime/1000.0f);
		//camera.direction[2] += mouseDelta[0] * (speed * (float) elapsedTime/1000.0f);

		//updated = true;
	}

	if (mouseDelta[1] > 0)
	{
		camera.direction[2] += mouseDelta[0] * (-speed * (float) elapsedTime/1000.0f);

		updated = true;
	}

	double normalizationScalar = 0;
	normalizationScalar += camera.direction[0] * camera.direction[0];
	normalizationScalar += camera.direction[1] * camera.direction[1];
	normalizationScalar += camera.direction[2] * camera.direction[2];
	normalizationScalar = std::sqrt(normalizationScalar);

	camera.direction[0] /= normalizationScalar;
	camera.direction[1] /= normalizationScalar;
	camera.direction[2] /= normalizationScalar;

	mouseDelta[0] = 0.0;
	mouseDelta[1] = 0.0;

	if (updated)
	{
		msgBus->sendMessage(Message(SetCameraPosition, camera.position));
		msgBus->sendMessage(Message(SetCameraDirection, camera.direction));
	}
}

CameraSystem::CameraSystem()
{
	setMessageCallback(KeyPress, (message_method_t) &CameraSystem::startMotion);
	setMessageCallback(KeyRelease, (message_method_t) &CameraSystem::stopMotion);
	setMessageCallback(SetMouseDelta, (message_method_t) &CameraSystem::setMouseDelta);

	setMessageCallback(SceneLoaded, (message_method_t) &CameraSystem::startRunning);
	setMessageCallback(SceneDestroyed, (message_method_t) &CameraSystem::stopRunning);

	keyBindings['W'] = Forward;
	keyBindings['S'] = Backward;
	keyBindings['A'] = Left;
	keyBindings['D'] = Right;
	keyBindings['Q'] = Up;
	keyBindings['E'] = Down;

	DEBUG("CAMERA_SYSTEM - Camera System Created");
}

CameraSystem::~CameraSystem()
{
	DEBUG("CAMERA_SYSTEM - Camera System Destroyed");
}

void CameraSystem::setMouseDelta(void * data)
{
	double * delta = (double *) data;

	mouseDelta[0] += delta[0];
	mouseDelta[1] += delta[1];
}

void CameraSystem::startMotion(void * data)
{
	long keyCode = reinterpret_cast<long> (data);

	auto action = keyBindings.find(keyCode);

	if (action == keyBindings.end())
		return;

	CameraMovementDirection dir = action->second;

	switch (dir)
	{
		case Forward:
			forward = true;
			break;
		case Backward:
			backward = true;
			break;
		case Up:
			up = true;
			break;
		case Down:
			down = true;
			break;
		case Left:
			left = true;
			break;
		case Right:
			right = true;
			break;
	};
}

void CameraSystem::stopMotion(void * data)
{
	long keyCode = reinterpret_cast<long> (data);

	auto action = keyBindings.find(keyCode);

	if (action == keyBindings.end())
		return;

	CameraMovementDirection dir = action->second;

	switch (dir)
	{
		case Forward:
			forward = false;
			break;
		case Backward:
			backward = false;
			break;
		case Up:
			up = false;
			break;
		case Down:
			down = false;
			break;
		case Left:
			left = false;
			break;
		case Right:
			right = false;
			break;
	};
}

void CameraSystem::startRunning(void * data)
{
	running = true;

	msgBus->sendMessage(Message(SetCameraPosition, camera.position));
}

void CameraSystem::stopRunning(void * data)
{
	running = false;
}
