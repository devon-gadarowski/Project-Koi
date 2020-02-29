
#include <CameraSystem.h>

#include <cmath>
#include <algorithm>

void CameraSystem::update(long elapsedTime)
{
	if (!running)
		return;

	bool updated = false;
	if (forward)
	{
		camera.position[0] += (camera.direction[0] * (speed * (float) elapsedTime/1000.0f));
		camera.position[1] += (camera.direction[1] * (speed * (float) elapsedTime/1000.0f));
		camera.position[2] += (camera.direction[2] * (speed * (float) elapsedTime/1000.0f));
		updated = true;
	}

	if (backward)
	{
		camera.position[0] += (camera.direction[0] * (-speed * (float) elapsedTime/1000.0f));
		camera.position[1] += (camera.direction[1] * (-speed * (float) elapsedTime/1000.0f));
		camera.position[2] += (camera.direction[2] * (-speed * (float) elapsedTime/1000.0f));
		updated = true;
	}

	if (up)
	{
		camera.position[0] = camera.position[0];
		camera.position[1] += (speed * (float) elapsedTime/1000.0f);
		camera.position[2] = camera.position[2];
		updated = true;
	}

	if (down)
	{
		camera.position[0] = camera.position[0];
		camera.position[1] += (-speed * (float) elapsedTime/1000.0f);
		camera.position[2] = camera.position[2];
		updated = true;
	}

	if (left)
	{
		camera.position[0] -= (camera.right[0] * (speed * (float) elapsedTime/1000.0f));
		camera.position[1] -= (camera.right[1] * (speed * (float) elapsedTime/1000.0f));
		camera.position[2] -= (camera.right[2] * (speed * (float) elapsedTime/1000.0f));
		updated = true;
	}

	if (right)
	{
		camera.position[0] += (camera.right[0] * (speed * (float) elapsedTime/1000.0f));
		camera.position[1] += (camera.right[1] * (speed * (float) elapsedTime/1000.0f));
		camera.position[2] += (camera.right[2] * (speed * (float) elapsedTime/1000.0f));
		updated = true;
	}

	if (mouseDelta[0] != 0 || mouseDelta[1] != 0)
	{
		camera.theta += mouseDelta[0] * (0.1 * (float) elapsedTime/1000.0f);
		camera.phi += mouseDelta[1] * (0.1 * (float) elapsedTime/1000.0f);

		camera.phi = std::clamp(camera.phi, 0.04f, 3.1f);

		camera.direction[0] = std::cos(camera.theta) * std::sin(camera.phi);
		camera.direction[1] = std::cos(camera.phi);
		camera.direction[2] = std::sin(camera.theta) * std::sin(camera.phi);

		camera.right[0] = std::cos(camera.theta + 3.14159 / 2);
		camera.right[1] = 0.0f;
		camera.right[2] = std::sin(camera.theta + 3.14159 / 2);

		mouseDelta[0] = 0.0;
		mouseDelta[1] = 0.0;

		updated = true;
	}

	if (updated)
	{
		msgBus->sendMessage(Message(SetCameraPosition, camera.position));
		msgBus->sendMessage(Message(SetCameraDirection, camera.direction));
	}
}

CameraSystem::CameraSystem()
{
	setMessageCallback(KeyPress, (message_method_t) &CameraSystem::onKeyPress);
	setMessageCallback(KeyRelease, (message_method_t) &CameraSystem::onKeyRelease);
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

	if (!running)
		return;

	mouseDelta[0] += delta[0];
	mouseDelta[1] += delta[1];
}

void CameraSystem::onKeyPress(void * data)
{
	long keyCode = reinterpret_cast<long> (data);

	if (keyCode == '`')
	{
		running = !running;
		DEBUG("CAMERA_SYSTEM - Pause key");
		return;
	}

	if (!running)
		return;

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

void CameraSystem::onKeyRelease(void * data)
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
	msgBus->sendMessage(Message(SetCameraDirection, camera.direction));
}

void CameraSystem::stopRunning(void * data)
{
	running = false;
}
