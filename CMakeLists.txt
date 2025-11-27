cmake_minimum_required(VERSION 3.25.1)
project(sprite_utils)

set(CMAKE_CXX_STANDARD 23)

#find_package(OpenSSL REQUIRED)
find_package(OpenCV REQUIRED)
add_executable(sprite_utils
        src/HelpCommand.cpp
        src/DrawCommand.cpp
        src/Main.cpp
        src/SpriteSheet.cpp
        src/SpriteSheetRow.cpp
        src/SpriteUtils.cpp
        src/SpriteUtilsArgs.cpp
        src/SpriteUtilsOptions.cpp
        src/Utils.cpp
        src/VersionCommand.cpp
)

target_include_directories(sprite_utils PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${OpenCV_INCLUDE_DIRS}
        
)

target_link_libraries(sprite_utils PRIVATE
        #OpenSSL::SSL
        #OpenSSL::Crypto
        ${OpenCV_LIBS}
)

