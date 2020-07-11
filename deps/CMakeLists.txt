#Glad
set(GLAD_PATH ${CMAKE_CURRENT_SOURCE_DIR}/glad)
set(im ${CMAKE_CURRENT_SOURCE_DIR}/glad)
add_library(glad ${GLAD_PATH}/glad.c)

add_subdirectory(glm)
add_subdirectory(imgui)
add_subdirectory(imgui_impl)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})