
#ifndef RENDER_SYSTEM_H
#define RENDER_SYSTEM_H

#include <system/Log.h>
#include <system/System.h>

#ifndef ANDROID

#include <render/DesktopContext.h>
#include <render/DesktopRenderer.h>
#include <render/GUI.h>

#else

#include <render/OVRContext.h>
#include <render/OVRRenderer.h>

#endif

#include <render/Scene3D.h>

class RenderSystem : public System
{
	public:
		void init();
		void update(long elapsedTime);

		void draw();

		RenderSystem() {}
		~RenderSystem();

#ifdef ANDROID
		RenderSystem(android_app * android_context) { this->android_context = android_context; }
#endif

	private:
		Context * context = nullptr;
		Renderer * renderer = nullptr;
		Scene * scene = nullptr;

#ifndef ANDROID
		GUI * gui = nullptr;
#else
    	android_app * android_context;
#endif

		bool paused = true;

		void loadScene(Message * msg);
		void onKeyPress(Message * msg);
		void onWindowFocus(Message * msg);
		void onCameraPositionUpdate(Message * msg);
		void onCameraDirectionUpdate(Message * msg);
};

#endif
