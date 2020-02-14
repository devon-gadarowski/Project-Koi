
#ifndef INPUT_SYSTEM_H
#define INPUT_SYSTEM_H

#include <SystemFramework.h>

#include <GLFW/glfw3.h>

class InputSystem : public System
{
	public:
		void update(long elapsedTime);

		InputSystem();
		~InputSystem();

	private:
		void onWindowCreate(void * data);

		static void keyCallback(GLFWwindow * window, int key, int scancode, int action, int mods);
		static void cursorPositionCallback(GLFWwindow * window, double xpos, double ypos);
		static void mouseButtonCallback(GLFWwindow * window, int button, int action, int mods);
		static void windowFocusCallback(GLFWwindow* window, int focused);

		std::unordered_map<int, bool> keyCodes;
		std::unordered_map<int, bool> mouseButtonCodes;
		double mousePosition [2];
		double mouseDelta [2];
};

#endif
