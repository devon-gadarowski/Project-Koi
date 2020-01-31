
#ifndef CAMERA_SYSTEM_H
#define CAMERA_SYSTEM_H

#include <SystemFramework.h>

#include <unordered_map>

struct Camera
{
	float position[3] = {0.0f, 0.0f, 0.0f};
	float direction[3] = {0.0f, 0.0f, -1.0f};;

	float up[3] = {0.0f, 1.0f, 0.0f};

	void setPosition(float x, float y, float z)
	{
		position[0] = x;
		position[1] = y;
		position[2] = z;
	}

	void setDirection(float x, float y, float z)
	{
		direction[0] = x;
		direction[1] = y;
		direction[2] = z;
	}
};

enum CameraMovementDirection
{
	Forward,
	Backward,
	Up,
	Down,
	Left,
	Right
};

class CameraSystem : public System
{
	public:
		void update(long elapsedTime);

		CameraSystem();
		~CameraSystem();

	private:
		Camera camera;
		std::unordered_map<long, CameraMovementDirection> keyBindings;

		bool running = false;

		float speed = 2.0f;
		bool forward = false;
		bool backward = false;
		bool up = false;
		bool down = false;
		bool left = false;
		bool right = false;

		double mouseDelta[2] = {0.0, 0.0};

		void startRunning(void * data);
		void stopRunning(void * data);
		void startMotion(void * data);
		void stopMotion(void * data);
		void setMouseDelta(void * data);
};

#endif
