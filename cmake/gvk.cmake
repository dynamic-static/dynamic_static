
include_guard()

include(FetchContent)

set(DST_GVK_VERSION 9a69ef9f2aad79df059efc33d0c96f0c8bf73e91)
set(DST_GVK_SOURCE_DIR "" CACHE STRING "Local GVK source directory")
set(DST_GVK_INSTALL_DIR "" CACHE STRING "Local GVK install directory")
set(DST_GVK_GIT_REPO_URL "https://github.com/dynamic-static/gvk.git")
set(gvk-ide-folder "${DST_IDE_FOLDER}/external/gvk" CACHE STRING "" FORCE)
set(gvk-build-tests OFF CACHE BOOL "" FORCE) 
if(DST_GVK_SOURCE_DIR)
    string(REPLACE "\\" "/" DST_GVK_SOURCE_DIR "${DST_GVK_SOURCE_DIR}")
    FetchContent_Declare(gvk SOURCE_DIR "${DST_GVK_SOURCE_DIR}")
    FetchContent_MakeAvailable(gvk)
else()
    if(DST_GVK_INSTALL_DIR)
        string(REPLACE "\\" "/" DST_GVK_INSTALL_DIR "${DST_GVK_INSTALL_DIR}")
        list(APPEND CMAKE_PREFIX_PATH "${DST_GVK_INSTALL_DIR}/cmake/")
        find_package(gvk)
        # # NOTE : The prebuilt GVK package comes with both Debug and Release artifacts...
        # #   this seperates them so they're installed individually based on configuration.
        # file(COPY "${DST_GVK_INSTALL_DIR}/cmake/" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/gvk/cmake/Debug/")
        # file(GLOB_RECURSE GVK_RELEASE_CONFIG_FILES "${CMAKE_CURRENT_BINARY_DIR}/gvk/cmake/Debug/*-release.cmake")
        # file(REMOVE ${GVK_RELEASE_CONFIG_FILES})
        # file(COPY "${DST_GVK_INSTALL_DIR}/cmake/" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/gvk/cmake/Release/")
        # file(GLOB_RECURSE GVK_DEBUG_CONFIG_FILES "${CMAKE_CURRENT_BINARY_DIR}/gvk/cmake/Release/*-debug.cmake")
        # file(REMOVE ${GVK_DEBUG_CONFIG_FILES})
        install(DIRECTORY "${DST_GVK_INSTALL_DIR}/include/" DESTINATION include/)
        install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/gvk/cmake/$<IF:$<CONFIG:Debug>,Debug,Release>/" DESTINATION cmake/)
        install(DIRECTORY "${DST_GVK_INSTALL_DIR}/bin/$<IF:$<CONFIG:Debug>,Debug,Release>/" DESTINATION bin/$<IF:$<CONFIG:Debug>,Debug,Release>/)
        install(DIRECTORY "${DST_GVK_INSTALL_DIR}/lib/$<IF:$<CONFIG:Debug>,Debug,Release>/" DESTINATION lib/$<IF:$<CONFIG:Debug>,Debug,Release>/)
    else()
        FetchContent_Declare(gvk GIT_REPOSITORY ${DST_GVK_GIT_REPO_URL} GIT_TAG ${DST_GVK_VERSION})
        FetchContent_MakeAvailable(gvk)
    endif()
endif()
