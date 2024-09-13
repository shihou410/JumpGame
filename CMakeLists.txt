cmake_minimum_required(VERSION 3.5.0)

# 指定 C++ 标准版本
set(CMAKE_CXX_STANDARD 20)  # 例如使用 C++17，可以改成 11、14、20 等
set(CMAKE_CXX_STANDARD_REQUIRED True)  # 强制使用该标准版本
set(CMAKE_CXX_EXTENSIONS OFF)  # 禁用 GNU 扩展

project(game2 VERSION 0.1.0 LANGUAGES C CXX)
# 查找SDL2包
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)
pkg_check_modules(SDL2_IMAGE REQUIRED SDL2_image)

# 查找box2d包
pkg_check_modules(BOX2D REQUIRED box2d)

add_executable(game2 main.cpp)
# 添加 SDL2 头文件路径
target_include_directories(game2 PRIVATE ${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS} ${BOX2D_INCLUDE_DIRS})

# 链接 SDL2 库
target_link_libraries(game2 ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES} ${BOX2D_INCLUDE_LIBRARIES})


