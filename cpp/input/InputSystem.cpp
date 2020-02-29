
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
}

void InputSystem::onWindowCreate(void * data)
{
	GLFWwindow * window = (GLFWwindow *) data;

	glfwSetWindowUserPointer(window, this);

	glfwSetKeyCallback(window, (GLFWkeyfun) &InputSystem::keyCallback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	if (glfwRawMouseMotionSupported())
		glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

	glfwSetCursorPosCallback(window, &InputSystem::cursorPositionCallback);
	glfwSetMouseButtonCallback(window, &InputSystem::mouseButtonCallback);

	glfwSetWindowFocusCallback(window, &InputSystem::windowFocusCallback);
}

void InputSystem::keyCallback(GLFWwindow * window, int key, int scancode, int action, int mods)
{
	InputSystem * inputSystem = (InputSystem *) glfwGetWindowUserPointer(window);

	if (action == GLFW_PRESS)
	{
		if (key == '`')
			inputSystem->toggleCursor(window);

		inputSystem->keyCodes[key] = true;
		inputSystem->msgBus->sendMessage(Message(KeyPress, reinterpret_cast<void *> (key)));
	}
	else if (action == GLFW_RELEASE)
	{
		inputSystem->keyCodes[key] = false;
		inputSystem->msgBus->sendMessage(Message(KeyRelease, reinterpret_cast<void *> (key)));
	}
}

void InputSystem::cursorPositionCallback(GLFWwindow* window, double xpos, double ypos)
{
	if (glfwGetWindowAttrib(window, GLFW_FOCUSED) != GLFW_TRUE)
		return;

	InputSystem * inputSystem = (InputSystem *) glfwGetWindowUserPointer(window);

	inputSystem->mouseDelta[0] = xpos - inputSystem->mousePosition[0];
	inputSystem->mouseDelta[1] = ypos - inputSystem->mousePosition[1];

	inputSystem->msgBus->sendMessageNow(Message(SetMouseDelta, inputSystem->mouseDelta));

	inputSystem->mousePosition[0] = xpos;
	inputSystem->mousePosition[1] = ypos;

	inputSystem->msgBus->sendMessageNow(Message(SetMouseCursor, inputSystem->mousePosition));
}

void InputSystem::mouseButtonCallback(GLFWwindow * window, int button, int action, int mods)
{
	InputSystem * inputSystem = (InputSystem *) glfwGetWindowUserPointer(window);

	if (action == GLFW_PRESS)
	{
		inputSystem->mouseButtonCodes[button] = true;
		inputSystem->msgBus->sendMessage(Message(MouseButtonPress, reinterpret_cast<void *> (button)));
	}
	else if (action == GLFW_RELEASE)
	{
		inputSystem->mouseButtonCodes[button] = false;
		inputSystem->msgBus->sendMessage(Message(MouseButtonRelease, reinterpret_cast<void *> (button)));
	}
}

void InputSystem::windowFocusCallback(GLFWwindow* window, int focused)
{
	InputSystem * inputSystem = (InputSystem *) glfwGetWindowUserPointer(window);

	inputSystem->msgBus->sendMessage(Message(SetWindowFocus, reinterpret_cast<void *> (focused)));
}
