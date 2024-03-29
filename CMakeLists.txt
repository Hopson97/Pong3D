cmake_minimum_required(VERSION 3.10)

#Set up project
project(Pong3D VERSION 1.0)

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


)

add_executable(${PROJECT_NAME} ${SOURCES})

#Set C++17
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_EXTENSIONS OFF)

#Set flags
if(MSVC)
  	target_compile_options(${PROJECT_NAME} PRIVATE 
    	/W4 /WX)
else()
  	target_compile_options(${PROJECT_NAME} PRIVATE 
		-Wall -Wextra -Werror -pedantic)		#Warning flags
endif()

#Set module path
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules" ${CMAKE_MODULE_PATH})

#Subdirectories
add_subdirectory(deps)

#Find libraries
find_package(Threads)
find_package(SFML REQUIRED audio network graphics window system)

#Finally
target_link_libraries(${PROJECT_NAME} 
    glad
    glm
    imgui
    imgui_impl
    Threads::Threads 
    ${SFML_LIBRARIES} 
    ${SFML_DEPENDENCIES}
    ${CMAKE_DL_LIBS}
)