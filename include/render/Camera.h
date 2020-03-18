#ifndef CAMERA_H
#define CAMERA_H

#include <vector>

#include <render/KoiVulkan.h>

#include <system/System.h>
#include <system/Log.h>
#include <render/Scene.h>
#include <render/KoiVector.h>

enum CameraMovementDirection
{
	Forward,
	Backward,
	Up,
	Down,
	Left,
	Right,
    Pause
};

struct CameraData
{
	Mat4 view;
	Mat4 proj;
};

struct Camera : public System
{
    uint32_t index = 0; 
    std::vector<UniformBuffer> buffers;

    CameraData data;

    float theta = 90.0f;
    float phi = 0.0f;

    Vec3 position = {0.0f, 0.0f, 1.0f};
    Vec3 direction = {0.0f, 0.0f, -1.0f};
    Vec3 upDir = {0.0f, 1.0f, 0.0f};

    VkExtent2D extent;
    float near = 0.1f;
    float far = 50.0f;
    float fov = 45.0f;

    bool running = false;

    std::unordered_map<long, CameraMovementDirection> keyBindings;

    float sensitivity = 4.0f;
    float speed = 2.0f;
    bool forward = false;
    bool backward = false;
    bool up = false;
    bool down = false;
    bool left = false;
    bool right = false;

    float mouseDelta[2] = {0.0, 0.0};

    void init();
    void update(long elapsedTime);

    void onKeyPress(Message * msg);
    void onKeyRelease(Message * msg);
    void setMouseDelta(Message * msg);

    void set(Vec3 position, Vec3 lookAt);
    void setPosition(Vec3 position);
    void setPosition(float x, float y, float z);
    void setLookAt(Vec3 lookAt);
    void setLookAt(float x, float y, float z);
    void updateBuffer();

    static VkDescriptorSetLayoutBinding getVkDescriptorSetLayoutBinding(uint32_t binding);
};

#endif