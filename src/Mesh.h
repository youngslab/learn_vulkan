#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

#include "Utilities.h"

struct Model {
  glm::mat4 model;
};

class Mesh {
public:
  Mesh();
  Mesh(VkPhysicalDevice newPhysicalDevice, vkx::Device newDevice,
       VkQueue transferQueue, VkCommandPool transferCommandPool,
       std::vector<Vertex> *vertices, std::vector<uint32_t> *indices,
       int newTexId);

  void setModel(glm::mat4 newModel);
  Model getModel();

  int getTexId();

  int getVertexCount();
  vkx::Buffer getVertexBuffer();

  int getIndexCount();
  vkx::Buffer getIndexBuffer();

  void destroyBuffers();

  ~Mesh();

private:
  Model model;
  int texId;

  int vertexCount;
  vkx::Buffer vertexBuffer;
  vkx::DeviceMemory vertexBufferMemory;

  int indexCount;
  vkx::Buffer indexBuffer;
  vkx::DeviceMemory indexBufferMemory;

  VkPhysicalDevice physicalDevice;
  vkx::Device device;

  void createVertexBuffer(VkQueue transferQueue,
			  VkCommandPool transferCommandPool,
			  std::vector<Vertex> *vertices);
  void createIndexBuffer(VkQueue transferQueue,
			 VkCommandPool transferCommandPool,
			 std::vector<uint32_t> *indices);
};

