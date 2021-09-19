cmake_minimum_required(VERSION 3.10)

project(VRender)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/")

set(SOURCE_FILES
    "src/scene/scene_hi.h"
    "src/vulkan/compute_shader.h"
    "src/vulkan/compute_shader.cpp"
    "src/vendors/stb/stb_image.h"
    "src/vendors/stb/stb_image.cpp"
    "src/vulkan/texture.h"
    "src/vulkan/texture.cpp"
    "src/vulkan/image.h"
    "src/vulkan/image.cpp"
    "src/vulkan/helpers.h"
    "src/vulkan/helpers.cpp"
    "src/vendors/spirv/spirv_reflect.h"
    "src/vendors/spirv/spirv_reflect.c"
    "src/managers/input_manager.h"
    "src/managers/input_manager.cpp"
    "src/input/input_map.h"
    "src/utils/timer.h"
    "src/rendering/camera.h"    
    "src/rendering/camera.cpp"
    "src/vulkan/descriptor.h"
    "src/vulkan/ubo.h"
    "src/vulkan/ubo.cpp"
    "src/managers/asset_manager.h"
    "src/managers/asset_manager.cpp"
    "src/vulkan/buffer.h"
    "src/vulkan/buffer.cpp"
    "src/vulkan/shader.h"
    "src/vulkan/shader.cpp"
    "src/debug/logger.h"
    "src/debug/logger.cpp"
    "src/debug/debug.h"
    "src/rendering/material.h"
    "src/managers/scene_manager.h"
    "src/managers/scene_manager.cpp"
    "src/managers/render_manager.h"
    "src/managers/render_manager.cpp"
    "src/engine/engine.h"
    "src/engine/engine.cpp"
    "src/vrender.h"
    "src/api.h"
    "src/vulkan/vulkan_app.h"
    "src/vulkan/vulkan_app.cpp"
    "src/main.cpp"
    )

#Build project files structure with vs filters
IF(MSVC)
    foreach(source IN LISTS SOURCE_FILES)
        get_filename_component(source_path "${source}" PATH)
        string(REPLACE "/" "\\" source_path_msvc "${source_path}")
        source_group("${source_path_msvc}" FILES "${source}")
    endforeach()
ENDIF(MSVC)

link_directories("extern/vulkan/Lib/")
link_directories("extern/GLFW/lib-vc2019/")
link_directories("extern/assimp/lib")

add_executable(VRender ${SOURCE_FILES})

#Copy dll
add_custom_command(TARGET VRender POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different
                   "${CMAKE_SOURCE_DIR}/extern/assimp/assimp-vc142-mt.dll"
                   "${PROJECT_BINARY_DIR}")

add_custom_command(TARGET VRender POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different
                   "${CMAKE_SOURCE_DIR}/extern/assimp/assimp-vc142-mt.dll"
                   "$<TARGET_FILE_DIR:VRender>")

execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink 
                "${CMAKE_SOURCE_DIR}/res"
                "${CMAKE_CURRENT_BINARY_DIR}/res")

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT VRender)

target_include_directories(VRender PRIVATE "src/")
target_include_directories(VRender PRIVATE "extern/vulkan/Include")
target_include_directories(VRender PRIVATE "extern/GLFW/include")
target_include_directories(VRender PRIVATE "extern/glm")
target_include_directories(VRender PRIVATE "extern/assimp/include")
target_include_directories(VRender PRIVATE "extern/SPIRV-Reflect")

target_link_libraries(VRender glfw3)
target_link_libraries(VRender vulkan-1)
target_link_libraries(VRender assimp-vc142-mt)

target_compile_definitions(VRender PRIVATE LIB WORKING_DIR="${PROJECT_BINARY_DIR}")