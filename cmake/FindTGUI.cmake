# Find TGUI library
find_package(PkgConfig QUIET)
pkg_check_modules(PC_TGUI QUIET tgui)

find_path(TGUI_INCLUDE_DIR
    NAMES TGUI/TGUI.hpp
    PATHS ${PC_TGUI_INCLUDE_DIRS}
    PATH_SUFFIXES include
)

find_library(TGUI_LIBRARY
    NAMES tgui
    PATHS ${PC_TGUI_LIBRARY_DIRS}
    PATH_SUFFIXES lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(TGUI
    REQUIRED_VARS TGUI_LIBRARY TGUI_INCLUDE_DIR
)

if(TGUI_FOUND)
    set(TGUI_LIBRARIES ${TGUI_LIBRARY})
    set(TGUI_INCLUDE_DIRS ${TGUI_INCLUDE_DIR})
endif()
