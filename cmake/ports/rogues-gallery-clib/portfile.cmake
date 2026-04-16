# header-only library
#vcpkg_from_github(
#    OUT_SOURCE_PATH SOURCE_PATH
#    REPO NoahBoddie/rgl-clib
#    REF HEAD #Replace with a specific commit if desired
#    SHA512 0
#    HEAD_REF master
#)


#include(${CMAKE_CURRENT_LIST_DIR}/porthelper.cmake)

#HANDLE_PORT(NoahBoddie RoguesGalleryClib HEAD 0 master)


#file(INSTALL ${SOURCE_PATH}/include/rgl-clib DESTINATION ${CURRENT_PACKAGES_DIR}/include)

#vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")