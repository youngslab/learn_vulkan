#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdexcept>
#include <vector>

#include "Utilities.h"

class VulkanRenderer
{
public:
	VulkanRenderer();

	int init(GLFWwindow * newWindow);
	void cleanup();

	~VulkanRenderer();

private:
	GLFWwindow * window;

	// Vulkan Components
	VkInstance instance;
	struct {
		VkPhysicalDevice physicalDevice;
		VkDevice logicalDevice;
	} mainDevice;
	VkQueue graphicsQueue;


	// Vulkan Functions
	// - Create Functions
	void createInstance();
	void createLogicalDevice();

	// - Get Functions
	void getPhysicalDevice();

	// - Support Functions
	// -- Checker Functions
	bool checkInstanceExtensionSupport(std::vector<const char*> * checkExtensions);
	bool checkDeviceSuitable(VkPhysicalDevice device);

	// -- Getter Functions
	QueueFamilyIndices getQueueFamilies(VkPhysicalDevice device);

};

