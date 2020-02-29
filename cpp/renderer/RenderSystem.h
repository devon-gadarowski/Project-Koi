
#ifndef RENDER_SYSTEM_H
#define RENDER_SYSTEM_H

#include <SystemFramework.h>
#include <RenderFramework.h>

class RenderSystem : public System
{
	public:
		void update(long elapsedTime);

		RenderSystem();
		~RenderSystem();

	private:
		GLFWwindow * window;
		RenderFramework::Context * context;
		RenderFramework::Renderer * renderer;
		RenderFramework::Scene3D * scene;
		RenderFramework::GUI * gui;

		bool initialized = false;
		bool sceneLoaded = false;
		bool paused = false;

		void initialize(void * data);
		void shutdown(void * data);
		void loadScene(void * data);
		void stopScene(void * data);
		void onKeyPress(void * data);
		void onWindowFocus(void * data);
		void onCameraPositionUpdate(void * data);
		void onCameraDirectionUpdate(void * data);
};

#endif
