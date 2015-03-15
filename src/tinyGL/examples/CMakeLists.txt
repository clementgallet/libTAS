FIND_PACKAGE(SDL REQUIRED)
include_directories(${SDL_INCLUDE_DIR})

add_executable(TinySDGLGears gears)
target_link_libraries(TinySDGLGears TinySDGL_shared ${SDL_LIBRARY})

add_executable(TinySDGLTriangle triangle)
target_link_libraries(TinySDGLTriangle TinySDGL_shared ${SDL_LIBRARY})
