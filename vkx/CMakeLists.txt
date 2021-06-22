
add_library(vkx ./src/Create.cpp ./src/Lifetime.cpp)
target_link_libraries(vkx PUBLIC Vulkan::Vulkan glfw::glfw)
target_include_directories(vkx PUBLIC ./include)
set_target_properties(vkx PROPERTIES
            CXX_STANDARD 17)
