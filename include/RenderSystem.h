
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
		RenderFramework::Context * context;
		RenderFramework::Renderer * renderer;
		RenderFramework::Scene3D * scene;
		RenderFramework::GUI * gui;

		bool initialized = false;
		bool sceneLoaded = false;
		bool paused = false;

		void init();
		void shutdown();
		void stopScene();

		void loadScene(Message * msg);
		void onKeyPress(Message * msg);
		void onWindowFocus(Message * msg);
		void onCameraPositionUpdate(Message * msg);
		void onCameraDirectionUpdate(Message * msg);
};

#endif
