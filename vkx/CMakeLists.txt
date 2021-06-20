
add_library(vkx INTERFACE)
target_link_libraries(vkx INTERFACE Vulkan::Vulkan glfw::glfw)
target_include_directories(vkx INTERFACE ./include)
