#include <algorithm>

#include <render/Camera.h>

void Camera::init()
{
	setMessageCallback(KeyPress, (message_method_t) &Camera::onKeyPress);
	setMessageCallback(KeyRelease, (message_method_t) &Camera::onKeyRelease);
	setMessageCallback(SetMouseDelta, (message_method_t) &Camera::setMouseDelta);

    keyBindings['W'] = Forward;
	keyBindings['S'] = Backward;
	keyBindings['A'] = Left;
	keyBindings['D'] = Right;
	keyBindings['Q'] = Up;
	keyBindings['E'] = Down;
    keyBindings['`'] = Pause;

    this->updateBuffer();

	DEBUG("CAMERA_SYSTEM - Camera System Created");
}

void Camera::update(long elapsedTime)
{
	if (!running)
		return;

	if (forward) this->position += this->direction * (speed * (float) elapsedTime/1000.0f);
	if (backward) this->position -= this->direction * (speed * (float) elapsedTime/1000.0f);
	if (up) this->position += this->upDir * (speed * (float) elapsedTime/1000.0f);
	if (down) this->position -= this->upDir * (speed * (float) elapsedTime/1000.0f);
	if (left) this->position += Vec3(glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), this->upDir) * glm::normalize(Vec4(this->direction.x, 0.0f, this->direction.z, 1.0f)) * (speed * (float) elapsedTime/1000.0f));
	if (right) this->position -= Vec3(glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), this->upDir) * glm::normalize(Vec4(this->direction.x, 0.0f, this->direction.z, 1.0f)) * (speed * (float) elapsedTime/1000.0f));
	if (mouseDelta[0] != 0) theta -= mouseDelta[0] * (sensitivity * (float) elapsedTime/1000.0f);
	if (mouseDelta[1] != 0)	{ phi -= mouseDelta[1] * (sensitivity * (float) elapsedTime/1000.0f); this->phi = std::clamp(this->phi, -89.0f, 89.0f);}
    if (mouseDelta[0] != 0 || mouseDelta[1] != 0) this->direction = glm::rotate(Mat4(1.0f), glm::radians(theta), this->upDir) * glm::rotate(Mat4(1.0f), glm::radians(phi), Vec3(0.0f, 0.0f, 1.0f)) * Vec4(1.0f, 0.0f, 0.0f, 1.0f);

    mouseDelta[0] = 0.0;
    mouseDelta[1] = 0.0;

    this->updateBuffer();
}

void Camera::set(Vec3 position, Vec3 direction)
{
    this->position = position;
    this->direction = direction;
    this->updateBuffer();
}

void Camera::setPosition(Vec3 position)
{
    this->position = position;
    this->updateBuffer();
}

void Camera::setPosition(float x, float y, float z)
{
    this->position.x = x;
    this->position.y = y;
    this->position.z = z;
    this->updateBuffer();
}

void Camera::updateBuffer()
{
    this->data.view = glm::lookAt(position, position + direction, upDir);
    this->data.proj = glm::perspective(glm::radians(this->fov), this->extent.width / (float) this->extent.height, this->near, this->far);
    this->data.proj[1][1] *= -1;

    memcpy(this->buffers[this->index++ % this->buffers.size()].data, &this->data, sizeof(CameraData));
}

void Camera::setMouseDelta(Message * msg)
{
	VectorMessage * delta = dynamic_cast<VectorMessage *> (msg);

	if (!running)
		return;

	mouseDelta[0] += delta->data[0];
	mouseDelta[1] += delta->data[1];
}

void Camera::onKeyPress(Message * msg)
{
	int keyCode = dynamic_cast<IntegerMessage *> (msg)->data;

    auto action = keyBindings.find(keyCode);

	if (action == keyBindings.end())
		return;

    CameraMovementDirection dir = action->second;

	if (dir == Pause)
        running = !running;

	if (!running)
		return;

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

void Camera::onKeyRelease(Message * msg)
{
	int keyCode = dynamic_cast<IntegerMessage *> (msg)->data;

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