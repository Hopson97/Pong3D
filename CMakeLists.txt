cmake_minimum_required(VERSION 3.10)

#Set up project
project(Pong3D VERSION 1.0)


include(${CMAKE_BINARY_DIR}/../conanbuildinfo.cmake)
conan_basic_setup()

#Set executable
set(SOURCES
    src/main.cpp
    src/Maths.cpp
    src/Mesh.cpp
    src/Settings.cpp
    src/GameObjects.cpp
    src/Screen.cpp
    src/ScreenMainMenu.cpp
    src/ScreenInGame.cpp
    
    src/GL/GLDebug.cpp 
    src/GL/Framebuffer.cpp 
    src/GL/Shader.cpp 
    src/GL/Texture.cpp 
    src/GL/VertexArray.cpp 

    src/imgui/imgui_impl_opengl3.cpp
    src/imgui/imgui_impl_sfml.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

#Set C++17
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_EXTENSIONS OFF)

#Set flags
if(MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "/O2")
    set(CMAKE_CXX_FLAGS_RELEASE "/Ox")
  	target_compile_options(${PROJECT_NAME} PRIVATE 
    	/W4)
else()
  	target_compile_options(${PROJECT_NAME} PRIVATE 
		-Wall -Wextra -pedantic)		
endif()


#Find libraries
find_package(Threads)

add_subdirectory(deps)

target_include_directories(
    ${PROJECT_NAME}
    PRIVATE
    deps
)

#Finally
target_link_libraries(${PROJECT_NAME} 
    glad 
    Threads::Threads 
    ${CMAKE_DL_LIBS}
    ${CONAN_LIBS}
)
