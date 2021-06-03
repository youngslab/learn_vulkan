
#include "vkx/raii.hpp"

#include "vkx/util.hpp"
#include <vulkan/vulkan_core.h>

#include <iostream>

namespace vkx {

auto Instance::Create(const VkInstanceCreateInfo *pCreateInfo,
		      const VkAllocationCallbacks *pAllocator) -> Instance {

  auto instance = std::shared_ptr<VkInstance>(
      new VkInstance(VK_NULL_HANDLE), [pAllocator](VkInstance *i) {
	vkDestroyInstance(*i, pAllocator);
	delete i;
      });

  auto res = vkCreateInstance(pCreateInfo, pAllocator, instance.get());
  if (res != VK_SUCCESS) {
    throw std::runtime_error("Failed to create an instance");
  }

  return Instance(instance);
}

namespace details {

auto CreateDebugUtilsMessengerEXT(
    VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkDebugUtilsMessengerEXT *pMessenger) -> VkResult {

  auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance, "vkCreateDebugUtilsMessengerEXT");

  if (func != nullptr) {
    return func(instance, pCreateInfo, pAllocator, pMessenger);
  } else {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}

auto DestroyDebugUtilsMessengerEXT(VkInstance instance,
				   VkDebugUtilsMessengerEXT messenger,
				   const VkAllocationCallbacks *pAllocator)
    -> void {

  auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance, "vkDestroyDebugUtilsMessengerEXT");

  if (func != nullptr) {
    func(instance, messenger, pAllocator);
  }
}

auto CreateDebugReportCallbackEXT(
    VkInstance instance, const VkDebugReportCallbackCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkDebugReportCallbackEXT *pCallback) -> VkResult {
  auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(
      instance, "vkCreateDebugReportCallbackEXT");

  if (func != nullptr) {
    return func(instance, pCreateInfo, pAllocator, pCallback);
  } else {
    std::cout << " not present !!\n";
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}

auto DestroyDebugReportCallbackEXT(VkInstance instance,
				   VkDebugReportCallbackEXT callback,
				   const VkAllocationCallbacks *pAllocator)
    -> void {
  // get function pointer to requested function, then cast to function pointer
  // for vkDestroyDebugReportCallbackEXT
  auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(
      instance, "vkDestroyDebugReportCallbackEXT");

  // If function found, execute
  if (func != nullptr) {
    func(instance, callback, pAllocator);
  }
}

} // namespace details

auto DebugUtilsMessenger::Create(
    Instance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator) -> DebugUtilsMessenger {

  auto debugUtilMessenger = std::shared_ptr<VkDebugUtilsMessengerEXT>(
      new VkDebugUtilsMessengerEXT(VK_NULL_HANDLE),
      [instance, pAllocator](VkDebugUtilsMessengerEXT *messenger) {
	details::DestroyDebugUtilsMessengerEXT(instance, *messenger,
					       pAllocator);
	delete messenger;
      });

  auto res = details::CreateDebugUtilsMessengerEXT(
      instance, pCreateInfo, pAllocator, debugUtilMessenger.get());
  if (res != VK_SUCCESS) {
    throw std::runtime_error("Failed to create an DebugUtilsMessenger");
  }

  return DebugUtilsMessenger(debugUtilMessenger);
}

auto DebugReportCallback::Create(
    Instance instance, const VkDebugReportCallbackCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator) -> DebugReportCallback {
  auto debugReportCallback = std::shared_ptr<VkDebugReportCallbackEXT>(
      new VkDebugReportCallbackEXT(VK_NULL_HANDLE),
      [instance, pAllocator](VkDebugReportCallbackEXT *callback) {
	details::DestroyDebugReportCallbackEXT(instance, *callback, pAllocator);
	delete callback;
      });

  auto res = details::CreateDebugReportCallbackEXT(
      instance, pCreateInfo, pAllocator, debugReportCallback.get());
  if (res != VK_SUCCESS) {
    throw std::runtime_error("Failed to create an DebugReportCallback.");
  }

  return DebugReportCallback(debugReportCallback);
}

auto Window::Create(uint32_t w, uint32_t h, std::string title) -> Window {

  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  auto window = glfwCreateWindow(w, h, title.c_str(), nullptr, nullptr);
  if (!window) {
    throw std::runtime_error("Failed to create a Surface.");
  }
  return Window(std::shared_ptr<GLFWwindow>(window, [](GLFWwindow *w) {
    std::cout << "delete window\n";
    glfwDestroyWindow(w);
    glfwTerminate();
  }));
}

auto Surface::Create(Instance instance, Window window,
		     const VkAllocationCallbacks *allocator) -> Surface {

  auto surface = std::shared_ptr<VkSurfaceKHR>(
      new VkSurfaceKHR(VK_NULL_HANDLE), [instance, allocator](VkSurfaceKHR *s) {
	vkDestroySurfaceKHR(instance, *s, allocator);
	delete s;
      });

  auto res =
      glfwCreateWindowSurface(instance, window, allocator, surface.get());
  if (res != VK_SUCCESS) {
    throw std::runtime_error("Failed to create a Surface.");
  }

  return Surface(surface);
}

auto Device::Create(VkPhysicalDevice const &physicalDevice,
		    const VkDeviceCreateInfo *pCreateInfo,
		    const VkAllocationCallbacks *pAllocator) -> Device {

  auto device = std::shared_ptr<VkDevice>(new VkDevice(VK_NULL_HANDLE),
					  [pAllocator](VkDevice *d) {
					    vkDestroyDevice(*d, pAllocator);
					    delete d;
					  });

  auto res = vkCreateDevice(physicalDevice, pCreateInfo, nullptr, device.get());
  if (res != VK_SUCCESS) {
    throw std::runtime_error("failed to create logical device!");
  }

  return Device(device);
}

auto Swapchain::Create(Device const &device,
		       const VkSwapchainCreateInfoKHR *pCreateInfo,
		       const VkAllocationCallbacks *pAllocator) -> Swapchain {

  auto swapchain = std::shared_ptr<VkSwapchainKHR>(
      new VkSwapchainKHR(VK_NULL_HANDLE), [device, pAllocator](auto p) {
	vkDestroySwapchainKHR(device, *p, pAllocator);
	delete p;
      });

  auto res =
      vkCreateSwapchainKHR(device, pCreateInfo, pAllocator, swapchain.get());
  if (res != VK_SUCCESS) {
    throw std::runtime_error("failed to create a Swapchain");
  }

  auto result = Swapchain(swapchain);
  result._extent = pCreateInfo->imageExtent;
  result._format = pCreateInfo->imageFormat;

  return result;
}

auto ImageView::Create(Device const &device,
		       const VkImageViewCreateInfo *createInfo,
		       const VkAllocationCallbacks *pAllocator) -> ImageView {

  auto imageView = std::shared_ptr<VkImageView>(
      new VkImageView(VK_NULL_HANDLE), [device, pAllocator](auto p) {
	vkDestroyImageView(device, *p, pAllocator);
	delete p;
      });

  auto res = vkCreateImageView(device, createInfo, pAllocator, imageView.get());
  if (res != VK_SUCCESS) {
    throw std::runtime_error("failed to create an ImageView");
  }

  return ImageView(imageView);
}

auto ShaderModule::Create(Device const &device,
			  const VkShaderModuleCreateInfo &createInfo,
			  const VkAllocationCallbacks *pAllocator)
    -> ShaderModule {
  auto shaderModule = std::shared_ptr<VkShaderModule>(
      new VkShaderModule(VK_NULL_HANDLE), [device, pAllocator](auto p) {
	vkDestroyShaderModule(device, *p, pAllocator);
	delete p;
      });

  auto res =
      vkCreateShaderModule(device, &createInfo, pAllocator, shaderModule.get());
  if (res != VK_SUCCESS) {
    throw std::runtime_error("failed to create an ShaderModule");
  }

  return ShaderModule(shaderModule);
}

} // namespace vkx
