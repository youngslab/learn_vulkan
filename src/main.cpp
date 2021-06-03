#define STB_IMAGE_IMPLEMENTATION
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdexcept>
#include <vector>
#include <iostream>

#include "VulkanRenderer.h"

VulkanRenderer vulkanRenderer;

int main() {
  // Create Window
  auto window = vkx::Window::Create(1366, 768, "vkapp");

  // Create Vulkan Renderer instance
  if (vulkanRenderer.init(window) == EXIT_FAILURE) {
    return EXIT_FAILURE;
  }

  float angle = 0.0f;
  float deltaTime = 0.0f;
  float lastTime = 0.0f;

  int helicopter =
      vulkanRenderer.createMeshModel("Models/12140_Skull_v3_L2.obj");

  // Loop until closed
  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    float now = glfwGetTime();
    deltaTime = now - lastTime;
    lastTime = now;

    angle += 10.0f * deltaTime;
    if (angle > 360.0f) {
      angle -= 360.0f;
    }

    glm::mat4 testMat = glm::rotate(glm::mat4(1.0f), glm::radians(angle),
				    glm::vec3(0.0f, 1.0f, 0.0f));
    testMat =
	glm::rotate(testMat, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    vulkanRenderer.updateModel(helicopter, testMat);

    vulkanRenderer.draw();
  }

  vulkanRenderer.cleanup();

  return 0;
}
