# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

set(CPP_DIR ${CMAKE_SOURCE_DIR}/src/main/cpp)
set(LIB_DIR ${CMAKE_SOURCE_DIR}/src/main/jniLibs/libs)

include_directories(  .
        ${CPP_DIR}
        ${CPP_DIR}/camera
        ${CPP_DIR}/imagereader
        ${CPP_DIR}/jniimp
        ${CPP_DIR}/mtcnn
        ${CPP_DIR}/opencv
        ${CPP_DIR}/opencv2
        ${CPP_DIR}/utils
        ${CPP_DIR}/mediacodec
        ${CPP_DIR}/ffmpeg
        ${CPP_DIR}/audio
        ${CPP_DIR}/face_lib)


add_library(libmtcnn SHARED IMPORTED )
set_target_properties(libmtcnn
        PROPERTIES IMPORTED_LOCATION
        ${LIB_DIR}/${ANDROID_ABI}/libmtcnn.so)

add_library(face_lib SHARED IMPORTED )
set_target_properties(face_lib
        PROPERTIES IMPORTED_LOCATION
        ${LIB_DIR}/${ANDROID_ABI}/libface_lib.so)

add_library(libopencv_java3
        SHARED
        IMPORTED)
set_target_properties(libopencv_java3
        PROPERTIES
        IMPORTED_LOCATION
        ${LIB_DIR}/${ANDROID_ABI}/libopencv_java3.so)

add_library(libffmpeg
        SHARED
        IMPORTED)
set_target_properties(libffmpeg
        PROPERTIES
        IMPORTED_LOCATION
        ${LIB_DIR}/${ANDROID_ABI}/libffmpeg.so)


# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

add_library( # Sets the name of the library.
        native-lib

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        src/main/cpp/native-lib.cpp
        ${CPP_DIR}/camera/camera_manager.cpp
        ${CPP_DIR}/camera/camera_listeners.cpp
        ${CPP_DIR}/camera/camera_utils.cpp
        ${CPP_DIR}/imagereader/image_reader.cpp
        ${CPP_DIR}/mediacodec/rtmp.cpp
        ${CPP_DIR}/audio/audio_common.cpp
        ${CPP_DIR}/audio/audio_player.cpp
        ${CPP_DIR}/audio/audio_recorder.cpp
        ${CPP_DIR}/audio/debug_utils.cpp
        ${CPP_DIR}/jniimp/impproc.cpp)

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log)

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
        native-lib
        android
        camera2ndk
        mediandk
        libmtcnn
        face_lib
        libopencv_java3
        libffmpeg
        OpenSLES
        atomic
        # Links the target library to the log library
        # included in the NDK.
        ${log-lib})