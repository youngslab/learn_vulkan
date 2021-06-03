

add_library(vkx ./src/raii.cpp ./src/tapi.cpp ./src/util.cpp)

target_include_directories(vkx PUBLIC ./include)

target_link_libraries(vkx PRIVATE glfw::glfw)
