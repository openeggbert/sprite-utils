cmake_minimum_required(VERSION 3.25.1)
project(sprite_utils)

set(CMAKE_CXX_STANDARD 23)

#find_package(OpenSSL REQUIRED)
# Only core/imgproc/imgcodecs are actually used (Mat/Rect/Scalar, rectangle/
# line/cvtColor, imread/imwrite) - request just those components instead of
# every OpenCV module, so this only depends on (and needs to find) the
# handful of shared libraries actually linked.
find_package(OpenCV REQUIRED COMPONENTS core imgproc imgcodecs)

# Sources shared by the CLI executable and the test executable (everything
# except each target's own main()).
set(SPRITE_UTILS_LIB_SOURCES
        src/HelpCommand.cpp
        src/DrawCommand.cpp
        src/ExtractCommand.cpp
        src/GifsCommand.cpp
        src/GifWriter.cpp
        src/PackCommand.cpp
        src/RestoreCommand.cpp
        src/SpriteSheet.cpp
        src/SpriteSheetRow.cpp
        src/SpriteUtils.cpp
        src/SpriteUtilsArgs.cpp
        src/SpriteUtilsOptions.cpp
        src/Utils.cpp
        src/VersionCommand.cpp
)

add_executable(sprite_utils
        src/Main.cpp
        ${SPRITE_UTILS_LIB_SOURCES}
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

enable_testing()

add_executable(sprite_utils_tests
        tests/TestMain.cpp
        tests/UtilsTests.cpp
        tests/SpriteSheetRowTests.cpp
        tests/SpriteSheetTests.cpp
        tests/SpriteUtilsArgsTests.cpp
        tests/GifWriterTests.cpp
        ${SPRITE_UTILS_LIB_SOURCES}
)

target_include_directories(sprite_utils_tests PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/tests
        ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(sprite_utils_tests PRIVATE
        ${OpenCV_LIBS}
)

add_test(NAME sprite_utils_tests COMMAND sprite_utils_tests)

