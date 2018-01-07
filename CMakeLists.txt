cmake_minimum_required(VERSION 2.8)
project(mcpelauncher)

include(ExternalProject)

enable_language(C ASM)

# Add CMake modules to path (used for finding libraries on macOS)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} -m32")

# For some reason, Clang forcefully exits when symbols are undefined
# LibZip 'requires' BZip2 for BZip2 decompression, but we don't use that
# Therefore, it is safe to ignore these linker errors and continue building
# _BZ2_bzCompress,_BZ2_bzCompressEnd,_BZ2_bzCompress,_BZ2_bzCompressEnd,_BZ2_bzCompressInit,_BZ2_bzDecompress,_BZ2_bzDecompressEnd,_BZ2_bzDecompressInit
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32 -undefined dynamic_lookup -Wno-unused-command-line-argument")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32 -undefined dynamic_lookup -Wno-unused-command-line-argument")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++11")
set(CMAKE_LINK_FLAGS "${CMAKE_LINK_FLAGS} -m32")
set(CMAKE_LIBRARY_ARCHITECTURE "i386-linux-gnu")

# Require and 'prefer' pthreads (this will fail if you do not have pthreads)
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)

find_package(Threads REQUIRED)
find_package(ZLIB REQUIRED)
find_package(OpenSSL REQUIRED)

# GLFW is statically compiled (so that we don't have to DYLD all the time)
# This means it requires OpenGL, Cocoa, CoreVideo and IOKit
if(APPLE)
  find_package(OpenGL REQUIRED)
  find_package(Cocoa REQUIRED)
  find_package(CoreVideo REQUIRED)
  find_package(IOKit REQUIRED)
endif(APPLE)

# GLFW 3.2.1 built from source using 32-bit static compilation
ExternalProject_Add(
    glfw
    URL "https://github.com/glfw/glfw/archive/3.2.1.tar.gz"
    URL_HASH SHA1=bee9cd9a8b4925184070ade5bc287c5635830eb0
    CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/ext/glfw" "-DBUILD_SHARED_LIBS=OFF" "-DCMAKE_C_FLAGS=-m32" "-DCMAKE_LINK_FLAGS=-m32"
)

# LibZip 1.4.0 built from source using 32-bit static compilation
ExternalProject_Add(
    zip
    URL "https://github.com/nih-at/libzip/archive/rel-1-4-0.tar.gz"
    URL_HASH SHA1=7b38ace94d806f200bd34d67e40c9924291b6745
    CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/ext/zip" "-DCMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS=\"-U _BZ2_bzCompress\"" "-DBUILD_SHARED_LIBS=OFF" "CMAKE_CXX_FLAGS=-m32" "-DCMAKE_C_FLAGS=-m32" "-DCMAKE_LINK_FLAGS=-m32"
)

# OSSP-UUID 1.6.2 built from source using 32-bit static compilation
ExternalProject_Add(
    uuid
    URL "https://mirrors.ocf.berkeley.edu/debian/pool/main/o/ossp-uuid/ossp-uuid_1.6.2.orig.tar.gz"
    URL_HASH SHA1=3e22126f0842073f4ea6a50b1f59dcb9d094719f
    CONFIGURE_COMMAND ${CMAKE_BINARY_DIR}/uuid-prefix/src/uuid/configure --disable-shared --enable-static --without-perl --without-php --without-pgsql --prefix=${CMAKE_BINARY_DIR}/ext/uuid
    BUILD_COMMAND make CFLAGS="-m32" LDFLAGS="-m32"
    BUILD_IN_SOURCE 1
)

# Add 'libs', epoll and hybris support (this will compile both)
add_subdirectory(libs)

# Add GLFW, LibZip and OSSP-UUID static libraries to linking
set(LIBRARIES ${CMAKE_BINARY_DIR}/ext/glfw/lib/libglfw3.a ${CMAKE_BINARY_DIR}/ext/zip/lib/libzip.a ${CMAKE_BINARY_DIR}/ext/uuid/lib/libuuid.a)

# Include RapidXML, Hybris (also epoll and glad) and the include directory which contains GLAD
include_directories(libs/rapidxml/ libs/hybris/include libs/glad/include include/)
include_directories(${CMAKE_BINARY_DIR}/ext/glfw/include ${CMAKE_BINARY_DIR}/ext/zip/include ${CMAKE_BINARY_DIR}/ext/uuid/include)
include_directories(${OPENSSL_INCLUDE_DIR} ${ZLIB_INCLUDE_DIRS} ${Threads_INCLUDE_DIR} ${OpenGL_INCLUDE_DIR} ${PNG_INCLUDE_DIRS})

set(CEF_ENABLED true)
set(PLAYAPI_ENABLED true)
if (NOT EXISTS ${CMAKE_SOURCE_DIR}/libs/cef)
    message(WARNING "CEF not found. Xbox Live login and APK download features will be disabled.\nTo set up CEF please run setup_cef.sh in the source directory.")
    set(CEF_ENABLED false)
    set(PLAYAPI_ENABLED false)
endif()
if (PLAYAPI_ENABLED)
    find_package(Protobuf)
    if (NOT Protobuf_FOUND)
        message(WARNING "Protobuf not found. Downloading the apk with a Google account will not be possible.")
        set(PLAYAPI_ENABLED false)
    endif()
endif()
if (PLAYAPI_ENABLED)
    add_subdirectory(gplay_api)
    set_target_properties(gplaydl PROPERTIES EXCLUDE_FROM_ALL TRUE)
endif()

set(SOURCE_FILES src/main.cpp src/common.cpp src/hook.cpp src/linux_appplatform.cpp src/minecraft/types.cpp src/minecraft/ImagePickingCallback.h src/minecraft/FilePickerSettings.h src/minecraft/Common.h src/minecraft/Xbox.h src/minecraft/string.cpp src/minecraft/string.h src/base64.cpp src/base64.h src/msa.cpp src/msa.h src/msa_token.h src/msa_network.cpp src/msa_network.h src/xboxlive.h src/xboxlive.cpp src/cll.cpp src/cll.h src/async_result_util.h src/extract.cpp src/extract.h src/path_helper.cpp src/path_helper.h)

set(SOURCE_FILES_CEF)
if (CEF_ENABLED)
    link_directories(${CMAKE_SOURCE_DIR}/libs/cef/lib/)
    set(SOURCE_FILES ${SOURCE_FILES} ${SOURCE_FILES_CEF} src/browser.cpp src/browser.h src/xbox_login_browser.h src/xbox_login_browser.cpp src/initial_setup_browser.cpp src/initial_setup_browser.h)
endif()
if (PLAYAPI_ENABLED)
    set(SOURCE_FILES ${SOURCE_FILES} gplay_api/src/config.h gplay_api/src/config.cpp src/google_login_browser.h src/google_login_browser.cpp src/google_play_helper.cpp src/google_play_helper.h)
endif()

add_executable(mcpelauncher ${SOURCE_FILES})
add_dependencies(mcpelauncher epoll glfw zip uuid hybris) # includes glad and epoll

if(APPLE)
  include_directories(${Cocoa_INCLUDE_DIRS} ${CoreVideo_INCLUDE_DIRS} ${IOKit_INCLUDE_DIRS})
  target_link_libraries(mcpelauncher curl crypto epoll hybris glad ${CMAKE_DL_LIBS} ${OPENGL_LIBRARIES} ${LIBRARIES} ${OPENSSL_LIBRARIES} ${ZLIB_LIBRARIES} ${Cocoa_LIBRARIES} ${CoreVideo_LIBRARIES} ${IOKit_LIBRARIES} Threads::Threads)
else(NOT APPLE)
  target_link_libraries(mcpelauncher curl crypto epoll hybris glad X11 Xrandr Xcursor GL Xxf86vm Xinerama ${CMAKE_DL_LIBS} ${LIBRARIES} ${OPENSSL_LIBRARIES} ${ZLIB_LIBRARIES} Threads::Threads)
endif(APPLE)


if (CEF_ENABLED)
    include_directories(libs/cef)
    target_link_libraries(mcpelauncher libcef.so libcef_dll_wrapper.a)
else()
    target_compile_definitions(mcpelauncher PRIVATE DISABLE_CEF)
endif()
if (PLAYAPI_ENABLED)
    target_link_libraries(mcpelauncher gplayapi)
else()
    target_compile_definitions(mcpelauncher PRIVATE DISABLE_PLAYAPI)
endif()

function (add_non_source_dir_custom_command command...)
    add_custom_command(TARGET mcpelauncher POST_BUILD
            COMMAND $<$<NOT:$<STREQUAL:$<TARGET_FILE_DIR:mcpelauncher>,${CMAKE_SOURCE_DIR}>>:${command}>)
endfunction()

add_custom_command(TARGET mcpelauncher POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
        $<TARGET_FILE_DIR:mcpelauncher>/src)
add_custom_command(TARGET mcpelauncher POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_SOURCE_DIR}/libs/cef/lib/libcef.so $<TARGET_FILE_DIR:mcpelauncher>/libcef.so)
add_custom_command(TARGET mcpelauncher POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_SOURCE_DIR}/libs/cef/bin/icudtl.dat $<TARGET_FILE_DIR:mcpelauncher>/icudtl.dat)
add_custom_command(TARGET mcpelauncher POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_SOURCE_DIR}/libs/cef/bin/snapshot_blob.bin $<TARGET_FILE_DIR:mcpelauncher>/snapshot_blob.bin)
add_custom_command(TARGET mcpelauncher POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_SOURCE_DIR}/libs/cef/bin/natives_blob.bin $<TARGET_FILE_DIR:mcpelauncher>/natives_blob.bin)
add_custom_command(TARGET mcpelauncher POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_SOURCE_DIR}/libs/cef/bin/icudtl.dat $<TARGET_FILE_DIR:mcpelauncher>/icudtl.dat)
add_non_source_dir_custom_command(${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_SOURCE_DIR}/src/initial_setup_resources $<TARGET_FILE_DIR:mcpelauncher>/src/initial_setup_resources)
add_non_source_dir_custom_command(${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_SOURCE_DIR}/libs $<TARGET_FILE_DIR:mcpelauncher>/libs)
add_non_source_dir_custom_command(${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_SOURCE_DIR}/mcpelauncher-icon.png $<TARGET_FILE_DIR:mcpelauncher>/mcpelauncher-icon.png)

install(TARGETS mcpelauncher
        RUNTIME DESTINATION share/mcpelauncher/bin/)
install(FILES libs/cef/lib/libcef.so libs/cef/bin/snapshot_blob.bin libs/cef/bin/natives_blob.bin libs/cef/bin/icudtl.dat start_mcpelauncher.sh
        DESTINATION share/mcpelauncher/bin
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ)
install(FILES mcpelauncher-icon.png
        DESTINATION share/mcpelauncher
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ)
install(DIRECTORY libs/cef/res
        DESTINATION share/mcpelauncher/libs/cef
        PATTERN "*"
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ)
install(DIRECTORY src/initial_setup_resources
        DESTINATION share/mcpelauncher/src
        PATTERN "*"
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ)
install(DIRECTORY libs/native
        DESTINATION share/mcpelauncher/libs
        PATTERN "*"
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ)
install(DIRECTORY DESTINATION bin/)
install(CODE "
execute_process(
        COMMAND ${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_INSTALL_PREFIX}/share/mcpelauncher/bin/start_mcpelauncher.sh ${CMAKE_INSTALL_PREFIX}/bin/mcpelauncher
)")
install(FILES mcpelauncher.desktop DESTINATION share/applications/)
install(FILES mcpelauncher-icon.png DESTINATION share/pixmaps/)
