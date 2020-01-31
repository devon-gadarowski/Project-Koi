

#ifndef CAMERA_FRAMEWORK_H
#define CAMERA_FRAMEWORK_H

struct Camera
{
	float position[3] = {0.0f, 0.0f, 0.0f};
	float direction[3] = {0.0f, 0.0f, 0.0f};

	void setPosition(int x, int y, int z)
	{
		position[0] = x;
		position[1] = y;
		position[2] = z;
	}

	void setDirection(int x, int y, int z)
	{
		direction[0] = x;
		direction[1] = y;
		direction[2] = z;
	}
};

#endif
