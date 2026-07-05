cmake_minimum_required(VERSION 3.31)
project(cli_manga_reader)

set(CMAKE_CXX_STANDARD 23)

find_package(PkgConfig REQUIRED)


add_executable(cli_manga_reader src/main.cpp
        src/renderer/renderer.hpp
        src/stb_image.h
        src/cli/cli.hpp
        src/stb_image_write.h
        src/stb_image_resize2.h
        src/cli/cli.cpp
        src/renderer/image.hpp
        src/backend/backend.hpp
        src/backend/backend.cpp
        src/renderer/image.cpp
        src/renderer/renderer.cpp
        src/backend/ImageData.hpp
        src/renderer/KittyRenderer.hpp
        src/backend/ImageData.cpp
        src/stb.cpp
        src/backend/ImageLoader.hpp
        src/backend/ImageCache.hpp
        src/backend/Navigator.hpp
        src/backend/ImageLoader.cpp
        src/backend/DirectoryScanner.hpp
        src/app/ViewerApp.hpp
        src/app/ViewerApp.cpp
)

target_include_directories(cli_manga_reader PRIVATE
        include
)

target_link_libraries(cli_manga_reader
)


target_link_directories(cli_manga_reader PRIVATE

)
