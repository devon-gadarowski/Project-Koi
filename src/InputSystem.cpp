
#include <InputSystem.h>

void InputSystem::update(long elapsedTime)
{

}

InputSystem::InputSystem()
{
	setMessageCallback(GLFWwindowCreated, (message_method_t) &InputSystem::onWindowCreate);

	DEBUG("INPUT_SYSTEM - Input System Created");
}

InputSystem::~InputSystem()
{
	DEBUG("INPUT_SYSTEM - Input System Destroyed");
}

void InputSystem::toggleCursor(GLFWwindow * window)
{
	if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);		
	else
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	if (glfwRawMouseMotionSupported() && glfwGetInputMode(window, GLFW_RAW_MOUSE_MOTION) == GLFW_TRUE)
		glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
	else
		glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

	double x, y;

	glfwGetCursorPos(window, &x, &y);

	mousePosition[0] = x;
	mousePosition[1] = y;
}

void InputSystem::onWindowCreate(Message * msg)
{
	GLFWwindow * window = (GLFWwindow *) (dynamic_cast<PointerMessage *> (msg))->data;

	glfwSetWindowUserPointer(window, this);

	glfwSetKeyCallback(window, (GLFWkeyfun) &InputSystem::keyCallback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	if (glfwRawMouseMotionSupported())
		glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

	glfwSetCursorPosCallback(window, &InputSystem::cursorPositionCallback);
	glfwSetMouseButtonCallback(window, &InputSystem::mouseButtonCallback);

	glfwSetWindowFocusCallback(window, &InputSystem::windowFocusCallback);

	toggleCursor(window);
}

void InputSystem::keyCallback(GLFWwindow * window, int key, int scancode, int action, int mods)
{
	InputSystem * inputSystem = (InputSystem *) glfwGetWindowUserPointer(window);

	if (action == GLFW_PRESS)
	{
		if (key == '`')
			inputSystem->toggleCursor(window);

		inputSystem->keyCodes[key] = true;
		inputSystem->msgBus->sendMessage(KeyPress, key);
	}
	else if (action == GLFW_RELEASE)
	{
		inputSystem->keyCodes[key] = false;
		inputSystem->msgBus->sendMessage(KeyRelease, key);
	}
}

void InputSystem::cursorPositionCallback(GLFWwindow * window, double xpos, double ypos)
{
	InputSystem * inputSystem = (InputSystem *) glfwGetWindowUserPointer(window);

	inputSystem->mouseDelta[0] = xpos - inputSystem->mousePosition[0];
	inputSystem->mouseDelta[1] = ypos - inputSystem->mousePosition[1];

	if (glfwGetWindowAttrib(window, GLFW_FOCUSED) == GLFW_TRUE && glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
		inputSystem->msgBus->sendMessageNow(SetMouseDelta, inputSystem->mouseDelta, 2);

	inputSystem->mousePosition[0] = xpos;
	inputSystem->mousePosition[1] = ypos;

	if (glfwGetWindowAttrib(window, GLFW_FOCUSED) == GLFW_TRUE)
		inputSystem->msgBus->sendMessageNow(SetMouseCursor, inputSystem->mousePosition, 2);
}

void InputSystem::mouseButtonCallback(GLFWwindow * window, int button, int action, int mods)
{
	InputSystem * inputSystem = (InputSystem *) glfwGetWindowUserPointer(window);

	if (action == GLFW_PRESS)
	{
		inputSystem->mouseButtonCodes[button] = true;
		inputSystem->msgBus->sendMessage(MouseButtonPress, button);
	}
	else if (action == GLFW_RELEASE)
	{
		inputSystem->mouseButtonCodes[button] = false;
		inputSystem->msgBus->sendMessage(MouseButtonRelease, button);
	}
}

void InputSystem::windowFocusCallback(GLFWwindow* window, int focused)
{
	InputSystem * inputSystem = (InputSystem *) glfwGetWindowUserPointer(window);

	inputSystem->msgBus->sendMessage(SetWindowFocus, focused);
}
