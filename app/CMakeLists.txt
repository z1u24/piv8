# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")
set(COMMON_CMAKE_ARGUMENTS "-std=c++11 -Werror -Wno-unused-result -mstackrealign -fexceptions -fno-builtin-stpcpy -fno-rtti -D_LIBCPP_ABI_VERSION=Cr -D_LIBCPP_ENABLE_NODISCARD -D_LIBCPP_ABI_UNSTABLE")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fuse-ld=lld")
#set(CMAKE_CXX_FLAGS "${COMMON_CMAKE_ARGUMENTS} -O3 -fvisibility=hidden -ffunction-sections -fno-data-sections")
set(CMAKE_CXX_FLAGS "${COMMON_CMAKE_ARGUMENTS} -g")
# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

include_directories(
        # V8 uses a custom STL compiled with the LIBCPP_ABI_UNSTABLE flag. To maintain the ABI
        # compatibility we use this same STL headers for building the runtime
        src/main/cpp/include/libc++
        src/main/cpp
        src/main/cpp/include
        src/main/cpp/v8_inspector
        # This folder contains V8's built-in natives and snapshot blobs
        src/main/jniLibs/${ANDROID_ABI}/include
)


add_library( # Sets the name of the library.
        native-lib

        # Sets the library as a shared library.
        SHARED

        src/main/cpp/v8_inspector/src/inspector/protocol/CSS.cpp
        src/main/cpp/v8_inspector/src/inspector/protocol/Console.cpp
        src/main/cpp/v8_inspector/src/inspector/protocol/DOM.cpp
        src/main/cpp/v8_inspector/src/inspector/protocol/Debugger.cpp
        src/main/cpp/v8_inspector/src/inspector/protocol/HeapProfiler.cpp
        src/main/cpp/v8_inspector/src/inspector/protocol/Log.cpp
        src/main/cpp/v8_inspector/src/inspector/protocol/Network.cpp
        src/main/cpp/v8_inspector/src/inspector/protocol/Overlay.cpp
        src/main/cpp/v8_inspector/src/inspector/protocol/Page.cpp
        src/main/cpp/v8_inspector/src/inspector/protocol/Profiler.cpp
        src/main/cpp/v8_inspector/src/inspector/protocol/Protocol.cpp
        src/main/cpp/v8_inspector/src/inspector/protocol/Runtime.cpp
        src/main/cpp/v8_inspector/src/inspector/protocol/Schema.cpp
        src/main/cpp/v8_inspector/src/inspector/utils/base64.cpp

        src/main/cpp/v8_inspector/src/inspector/custom-preview.cc
        src/main/cpp/v8_inspector/src/inspector/injected-script.cc
        src/main/cpp/v8_inspector/src/inspector/inspected-context.cc
        src/main/cpp/v8_inspector/src/inspector/remote-object-id.cc
        src/main/cpp/v8_inspector/src/inspector/search-util.cc
        src/main/cpp/v8_inspector/src/inspector/string-16.cc
        src/main/cpp/v8_inspector/src/inspector/string-util.cc
        src/main/cpp/v8_inspector/src/inspector/v8-console.cc
        src/main/cpp/v8_inspector/src/inspector/v8-console-agent-impl.cc
        src/main/cpp/v8_inspector/src/inspector/v8-console-message.cc
        src/main/cpp/v8_inspector/src/inspector/v8-debugger.cc
        src/main/cpp/v8_inspector/src/inspector/v8-debugger-agent-impl.cc
        src/main/cpp/v8_inspector/src/inspector/v8-debugger-script.cc
        src/main/cpp/v8_inspector/src/inspector/v8-heap-profiler-agent-impl.cc
        src/main/cpp/v8_inspector/src/inspector/v8-inspector-impl.cc
        src/main/cpp/v8_inspector/src/inspector/v8-inspector-session-impl.cc
        src/main/cpp/v8_inspector/src/inspector/v8-profiler-agent-impl.cc
        src/main/cpp/v8_inspector/src/inspector/v8-regex.cc
        src/main/cpp/v8_inspector/src/inspector/v8-runtime-agent-impl.cc
        src/main/cpp/v8_inspector/src/inspector/v8-schema-agent-impl.cc
        src/main/cpp/v8_inspector/src/inspector/v8-stack-trace-impl.cc
        src/main/cpp/v8_inspector/src/inspector/v8-value-utils.cc
        src/main/cpp/v8_inspector/src/inspector/value-mirror.cc
        src/main/cpp/v8_inspector/src/inspector/wasm-translation.cc


        src/main/cpp/util.cpp
        src/main/cpp/util.h
        src/main/cpp/log.h
        )


set(NATIVES_BLOB_INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/include)

set_target_properties(
        native-lib
        PROPERTIES LINK_FLAGS -Wl,--allow-multiple-definition -Wl,--exclude-libs=ALL -Wl,--gc-sections
        INTERFACE_INCLUDE_DIRECTORIES NATIVES_BLOB_INCLUDE_DIRECTORIES
)


# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log )
#target_link_libraries(native-lib ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libzip.a)
target_link_libraries(native-lib ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libv8_base.a)
target_link_libraries(native-lib ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libv8_external_snapshot.a)
target_link_libraries(native-lib ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libv8_init.a)
target_link_libraries(native-lib ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libv8_initializers.a)
target_link_libraries(native-lib ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libv8_libplatform.a)
target_link_libraries(native-lib ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libv8_libsampler.a)
target_link_libraries(native-lib ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libv8_libbase.a)

target_link_libraries(native-lib ${PROJECT_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libc++.a)
target_link_libraries(native-lib ${PROJECT_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libc++abi.a)

if("${ANDROID_ABI}" MATCHES "armeabi-v7a" OR "${ANDROID_ABI}" MATCHES "x86")
    # On API Level 19 and lower we need to link with android_support
    # because it contains some implementation of functions such as "strtoll" and "strtoul"
    target_link_libraries(native-lib ${ANDROID_NDK_ROOT}/sources/cxx-stl/llvm-libc++/libs/${ANDROID_ABI}/libandroid_support.a)
endif()

target_link_libraries( # Specifies the target library.
        native-lib
        # Links the target library to the log library
        # included in the NDK.
        ${log-lib}
        android
        )
