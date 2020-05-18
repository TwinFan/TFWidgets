# TFWidgets build script.
#

cmake_minimum_required(VERSION 3.9)
project(TFWidgets VERSION 0.1.0 DESCRIPTION "TFWidgets Example Plugin")

# By default, use optimized release configuration.
if (NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
endif ()

set(CMAKE_CXX_STANDARD 17)

# Set include directories used by our code and dependencies.
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/..")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}")
include_directories("${CMAKE_SOURCE_DIR}/../clip")
include_directories("${CMAKE_SOURCE_DIR}/Lib/XPSDK301/CHeaders/XPLM")
message("CMAKE_CURRENT_SOURCE_DIR = "(${CMAKE_CURRENT_SOURCE_DIR}))
message("CMAKE_SOURCE_DIR = "(${CMAKE_SOURCE_DIR}))

# Specify library search locations.
if (WIN32)
    list(APPEND CMAKE_LIBRARY_PATH "${CMAKE_CURRENT_SOURCE_DIR}/Lib/XPSDK301/Libraries/Win")
elseif (APPLE)
    list(APPEND CMAKE_FRAMEWORK_PATH "${CMAKE_CURRENT_SOURCE_DIR}/Lib/XPSDK301/Libraries/Mac")
elseif (UNIX)

endif ()

# message (STATUS "CMAKE_LIBRARY_PATH" = ${CMAKE_LIBRARY_PATH})

# Enable all X-Plane SDK APIs up to the newest version.
add_definitions(-DXPLM200=1 -DXPLM210=1 -DXPLM300=1 -DXPLM301=1)

# Define platform macros.
add_definitions(-DAPL=$<BOOL:${APPLE}> -DIBM=$<BOOL:${WIN32}> -DLIN=$<AND:$<BOOL:${UNIX}>,$<NOT:$<BOOL:${APPLE}>>>)

# Enable stricter warnings and then disable some we are not interested in.
# For XPMP2 compile, we don't need to be warned about our self-defined depreciations
if (WIN32)
    add_compile_options(/wd4996 /wd4068)
else()
    add_compile_options(-Wall -Wshadow -Wfloat-equal -Wextra -Wno-deprecated-declarations -Wno-unknown-pragmas)

    # Force-enable exception support. This is most likely redundant, although for C
    # code the default is the opposite. Since we are mixing C++ and C libraries,
    # safer to set it on?
    add_compile_options(-fexceptions -fpermissive)

    # On UNIX systems this makes symbols non-exported by default. On Windows this
    # option is simply ignored, since symbol visibility works differently there.
    add_compile_options(-fvisibility=hidden)
endif()


# Debug build?
if(CMAKE_BUILD_TYPE MATCHES "Debug")
    add_compile_options(-DDEBUG=1)
	set(XPMP2_DEBUG ON)
else()
	set(XPMP2_DEBUG OFF)

    if (UNIX)
        # Use position-independent code and highest optimization level (FPS!).
        add_compile_options(-O3 -fPIC)
    endif()
endif()

################################################################################
# Source groups
################################################################################
set(Header_Files
    ../TFW.h
    ../TFWGraphics.h
    ../clip/clip_lock_impl.h
    ../clip/clip.h
)
source_group("Header Files" FILES ${Header_Files})

set(Source_Files
    ../TFW.cpp
    ../TFWGraphics.cpp
    ../clip/image.cpp
    ../clip/clip.cpp
    TFWExample.cpp
)

################################################################################
# clip
################################################################################
if(WIN32)
    add_definitions(-D_SCL_SECURE_NO_WARNINGS)
    list(APPEND Source_Files ../clip/clip_win.cpp)
elseif(APPLE)
#    add_definitions(-fobjc-arc)
    list(APPEND Source_Files ../clip/clip_osx.mm)
elseif(UNIX)
    include(CheckIncludeFiles)
    check_include_files(xcb/xcb.h HAVE_XCB_XLIB_H)

    if(HAVE_XCB_XLIB_H)
        add_definitions(-DHAVE_XCB_XLIB_H)

        list(APPEND Source_Files ../clip/clip_x11.cpp)
    else()
        list(APPEND Source_Files ../clip/clip_none.cpp)
    endif()
else()
    list(APPEND CLIP_SOURCES ../clip/clip_none.cpp)
endif()


source_group("Source Files" FILES ${Source_Files})

set(ALL_FILES  ${Header_Files} ${Source_Files})

add_library(TFWidgets SHARED ${ALL_FILES})

target_compile_features(TFWidgets PUBLIC cxx_std_17)

################################################################################
# Platform specifics / linking
################################################################################
if (APPLE)
    target_compile_options(TFWidgets PUBLIC -mmacosx-version-min=10.10)
    target_link_libraries(TFWidgets -mmacosx-version-min=10.10)
endif ()

# Link OpenGL and OpenAL related libraries.
find_package(OpenGL REQUIRED)  # apt install freeglut3-dev
if ( OpenGL_FOUND )
    include_directories( ${OpenGL_INCLUDE_DIRS} )
    target_link_libraries( TFWidgets ${OpenGL_LIBRARIES} )
endif( OpenGL_FOUND )

# find_package(GLUT REQUIRED)  # apt install freeglut3-dev
# if ( GLUT_FOUND )
#  include_directories( ${GLUT_INCLUDE_DIRS} )
#  target_link_libraries( TFWidgets ${GLUT_LIBRARIES} )
# endif( GLUT_FOUND )

# Link X-Plane plugin system libraries. They are only provided for OS X and Windows.
if (WIN32 OR APPLE)
    find_library(XPLM_LIBRARY NAMES XPLM XPLM_64.lib)
    target_link_libraries(TFWidgets ${XPLM_LIBRARY})
#	find_library(XPWIDGETS_LIBRARY NAMES XPWidgets XPWidgets_64.lib)
#    target_link_libraries(TFWidgets ${XPLM_LIBRARY} ${XPWIDGETS_LIBRARY})
endif ()


# Link library for dynamic loading of shared objects on UNIX systems.
if (UNIX)
    find_library(DL_LIBRARY dl)
    target_link_libraries(TFWidgets ${DL_LIBRARY})
endif ()

# Link OS X core system libraries.
if (APPLE)
    find_library(IOKIT_LIBRARY IOKit)
    find_library(CORE_FOUNDATION_LIBRARY CoreFoundation)
    find_library(GLUT_LIBRARY GLUT)
    find_library(OpenGL_LIBRARY OpenGL)
    find_library(Cocoa_LIBRARY Cocoa)

    target_link_libraries(TFWidgets ${IOKIT_LIBRARY} ${CORE_FOUNDATION_LIBRARY} ${GLUT_LIBRARY})
    target_link_libraries(TFWidgets ${OpenGL_LIBRARY} ${Cocoa_LIBRARY})
endif ()

if (WIN32)
    # Unlike OS X and Linux we build standard libraries statically since X-Plane does not provide them.
    # mingw only? - target_link_libraries(TFWidgets -static-libgcc -static-libstdc++)
    target_link_libraries(TFWidgets opengl32)
    # MSYS2 for some reason always links against winpthread, so we have to work around it by statically linking the
    # entire plugin. This allows building the plugin nativaly on Windows.
    # mingw only? - target_link_libraries(TFWidgets -static)
elseif (APPLE)
    # Restrict set of symbols exported from the plugin. This reduces changes of
    # conflict with other plugins, in particular ones with Lua interpreter
    # embedded.
    target_link_libraries(TFWidgets "-exported_symbols_list ${CMAKE_SOURCE_DIR}/TFWExample.sym_mac")
	target_link_libraries(TFWidgets "-framework XPLM -framework XPWidgets -ldl")
elseif (UNIX)
    # Specify additional runtime search laths for dynamically-linked libraries.
    # Restrict set of symbols exported from the plugin. This reduces changes of
    # conflict with other plugins, in particular ones with Lua interpreter
    # embedded.
    message(${CMAKE_CURRENT_SOURCE_DIR})
    list(APPEND CMAKE_LIBRARY_PATH "${CMAKE_CURRENT_SOURCE_DIR}/Lib/xplanemp")
    target_link_libraries(TFWidgets -Wl,--version-script -Wl,${CMAKE_SOURCE_DIR}/TFWExample.sym)
endif ()

set_target_properties(TFWidgets PROPERTIES PREFIX "")
if (WIN32)
   set_target_properties(TFWidgets PROPERTIES OUTPUT_NAME "TFWExample-win")
elseif (APPLE)
   set_target_properties(TFWidgets PROPERTIES OUTPUT_NAME "TFWExample-mac")
elseif (UNIX)
   set_target_properties(TFWidgets PROPERTIES OUTPUT_NAME "TFWExample-lin")
endif ()
set_target_properties(TFWidgets PROPERTIES SUFFIX ".xpl")
