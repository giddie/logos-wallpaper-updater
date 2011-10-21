###
# Copyright (c) 2008, Paul Gideon Dann
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. The name of the author may not be used to endorse or promote products
#    derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
# NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
###

cmake_minimum_required(VERSION 2.6.2)
project(logos-wallpaper)
set(APP_LONGNAME "Logos Wallpaper Updater")
set(APP_VERSION "1.3")

# If no build type is specified, default to "Release"
if (NOT CMAKE_BUILD_TYPE)
  set (CMAKE_BUILD_TYPE Release)
endif (NOT CMAKE_BUILD_TYPE)

#################################################
# Configuration
#################################################
find_package(Qt4 REQUIRED)
set(QT_USE_QTNETWORK 1)
include(${QT_USE_FILE})

#################################################
# Includes, Defines, and Flags
#################################################
include_directories(${CMAKE_CURRENT_BINARY_DIR})
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions")

#################################################
# Targets
#################################################
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Licence.txt
               ${CMAKE_CURRENT_BINARY_DIR}/Licence.txt COPYONLY)

file(GLOB APP_UIS source/*.ui)
file(GLOB APP_SOURCES source/*.cpp)
file(GLOB APP_QRCS resources/*.qrc)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/source/defines.h.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/defines.h)

if (WIN32)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/win/installer.nsi.cmake
                 ${CMAKE_CURRENT_BINARY_DIR}/installer.nsi @ONLY)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/win/qt.conf
                 ${CMAKE_CURRENT_BINARY_DIR}/qt.conf COPYONLY)
  set(APP_WINDOWS_RESFILE ${CMAKE_CURRENT_BINARY_DIR}/resources.obj)
  add_custom_command(OUTPUT ${APP_WINDOWS_RESFILE}
    COMMAND windres.exe resources.rc ${APP_WINDOWS_RESFILE}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/win
  )
endif (WIN32)

qt4_wrap_ui(APP_UIS_H ${APP_UIS})
qt4_add_resources(APP_RESOURCES ${APP_QRCS})
qt4_automoc(${APP_SOURCES})

add_executable(${CMAKE_PROJECT_NAME} WIN32
  ${APP_UIS_H}
  ${APP_SOURCES}
  ${APP_RESOURCES}
  ${APP_WINDOWS_RESFILE}
)
target_link_libraries(${CMAKE_PROJECT_NAME}
  ${QT_LIBRARIES}
)

if (WIN32)
  # Suppress warnings when compiling with GCC 4.3 in Windows
  # See GCC Bug 34749
  set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES
    COMPILE_FLAGS -Wno-attributes
    LINK_FLAGS -Wl,--enable-auto-import)
endif (WIN32)

if (APPLE)
  set(TEMP_BUNDLE ${CMAKE_CURRENT_BINARY_DIR}/bundle)
  set(REAL_BUNDLE ${CMAKE_CURRENT_BINARY_DIR}/${APP_LONGNAME}.app)
  set(EXEC_DIR ${TEMP_BUNDLE}/Contents/MacOS)
  add_custom_target(bundle
    COMMAND rm -rf ${TEMP_BUNDLE}
    COMMAND cp -R ${CMAKE_CURRENT_SOURCE_DIR}/mac/bundle ${TEMP_BUNDLE}
    COMMAND mkdir -p ${EXEC_DIR}
    COMMAND cp ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME} ${EXEC_DIR}
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/mac/qtbundle ${TEMP_BUNDLE}
    COMMAND rm -rf ${REAL_BUNDLE}
    COMMAND mv ${TEMP_BUNDLE} ${REAL_BUNDLE}
  )
  add_dependencies(bundle ${CMAKE_PROJECT_NAME})

  set(IMAGE_DIR ${CMAKE_CURRENT_BINARY_DIR}/image)
  set(FINAL_IMAGE ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}.dmg)
  add_custom_target(package
    COMMAND echo "Building disk image..."
    COMMAND rm -rf ${IMAGE_DIR}
    COMMAND mkdir ${IMAGE_DIR}
    COMMAND cp -R ${REAL_BUNDLE} ${IMAGE_DIR}
    COMMAND cp -R ${CMAKE_BINARY_DIR}/Licence.txt ${IMAGE_DIR}
    COMMAND hdiutil create -ov -srcfolder ${IMAGE_DIR} -format UDBZ
                           -volname ${APP_LONGNAME} ${FINAL_IMAGE}
    COMMAND rm -rf ${IMAGE_DIR}
  )
  add_dependencies(package bundle)
endif (APPLE)
