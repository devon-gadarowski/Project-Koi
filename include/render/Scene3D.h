#ifndef SCENE3D_H
#define SCENE3D_H

#include <vector>

#include <render/KoiVector.h>
#include <render/Context.h>
#include <render/Renderer.h>
#include <render/Scene.h>
#include <render/Model.h>
#include <render/Camera.h>

struct ModelData
{
    uint32_t _id;
    std::string name;
    uint32_t instanceCount;
};

struct DirectionalLightData
{
    alignas(16) Vec3 direction = {1.0f, 1.0f, 1.0f};
    alignas(16) Vec3 ambient = {0.1f, 0.1f, 0.1f};
    alignas(16) Vec3 diffuse = {0.5f, 0.5f, 0.5f};
    alignas(16) Vec3 specular = {1.0f, 1.0f, 1.0f};
};

struct DirectionalLight
{
    uint32_t index = 0; 
    std::vector<UniformBuffer> buffers;

    DirectionalLightData data;

    static VkDescriptorSetLayoutBinding getVkDescriptorSetLayoutBinding(uint32_t binding);
};

class Scene3D : public Scene
{
    public:
    Camera camera;
    DirectionalLight light;

    std::vector<ModelBase *> models;

    Scene3D(Context * context, Renderer * renderer);
    ~Scene3D();

    void init();
    void update(long elapsedTime);
    void draw(VkCommandBuffer commandbuffer);

    void updateLighting(Message * message);
    void getModelData(Message * message);
    void addModel(Message * message);
};

#endif