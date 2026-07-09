cmake_minimum_required(VERSION 3.31)
project(cli_manga_reader)

set(CMAKE_CXX_STANDARD 23)

find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)


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
        src/backend/imageData.hpp
        src/renderer/kittyRenderer.hpp
        src/backend/imageData.cpp
        src/stb.cpp
        src/backend/imageLoader.hpp
        src/backend/imageCache.hpp
        src/backend/imageLoadWorker.hpp
        src/backend/imageLoadWorker.cpp
        src/backend/navigator.hpp
        src/backend/imageLoader.cpp
        src/backend/directoryScanner.hpp
        src/app/viewerApp.hpp
        src/app/viewerApp.cpp
        src/backend/directoryScanner.cpp
        src/backend/directoryBrowser/directoryBrowser.hpp
)

target_include_directories(cli_manga_reader PRIVATE
        include
)

target_link_libraries(cli_manga_reader
        PRIVATE Threads::Threads
)


target_link_directories(cli_manga_reader PRIVATE

)
