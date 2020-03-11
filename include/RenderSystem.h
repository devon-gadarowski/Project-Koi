
#ifndef RENDER_SYSTEM_H
#define RENDER_SYSTEM_H

#include <system/Log.h>
#include <system/System.h>

#include <render/DesktopContext.h>
#include <render/DesktopRenderer.h>
#include <render/Scene3D.h>
#include <render/GUI.h>

class RenderSystem : public System
{
	public:
		void init();
		void update(long elapsedTime);

		void draw();

		RenderSystem() {}
		~RenderSystem();

	private:
		DesktopContext * context;
		DesktopRenderer * renderer;
		Scene * scene;
		GUI * gui;

		bool paused = true;

		void loadScene(Message * msg);
		void onKeyPress(Message * msg);
		void onWindowFocus(Message * msg);
		void onCameraPositionUpdate(Message * msg);
		void onCameraDirectionUpdate(Message * msg);
};

#endif
