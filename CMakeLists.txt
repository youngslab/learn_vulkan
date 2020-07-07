
cmake_minimum_required(VERSION 3.1)

project(learn_vulkan) 

if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake") 
  message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan") 
  file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/master/conan.cmake" "${CMAKE_BINARY_DIR}/conan.cmake") 
endif() 

include(${CMAKE_BINARY_DIR}/conan.cmake)
conan_cmake_run(CONANFILE conanfile.txt
    PROFILE ${CMAKE_CONNAN_PROFILE}
    BASIC_SETUP CMAKE_TARGETS
    BUILD missing)
  
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_BINARY_DIR})

# find_package(vulkan REQUIRED)
find_package(glm REQUIRED)
find_package(glfw REQUIRED)
find_package(fmt REQUIRED)
find_package(Vulkan REQUIRED)

# add_subdirectory(3rd_party)

add_executable(learn_vulkan src/main.cpp src/VulkanRenderer.cpp src/Mesh.cpp)

target_link_libraries(learn_vulkan Vulkan::Vulkan glm::glm glfw::glfw fmt::fmt)

target_include_directories(learn_vulkan 
  PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

set_target_properties(learn_vulkan PROPERTIES
            CXX_STANDARD 17)
