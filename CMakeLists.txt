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
project(logos-monthly)

#################################################
# Configuration
#################################################
find_package(Qt4 REQUIRED)
include(${QT_USE_FILE})


#################################################
# Includes and Defines
#################################################
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(
  ${QT_INCLUDE_DIR}
  ${QT_QTNETWORK_INCLUDE_DIR}
)


#################################################
# Targets
#################################################
file(GLOB APP_UIS source/*.ui)
file(GLOB APP_SOURCES source/*.cpp)
file(GLOB APP_QRCS resources/*.qrc)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/source/defines.h.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/defines.h)

if (APPLE)
  file(GLOB APP_MAC_RESOURCES mac/Resources/*)
  set_source_files_properties(
    ${APP_MAC_RESOURCES}
    PROPERTIES
    MACOSX_PACKAGE_LOCATION Resources
  )
  file(GLOB APP_MAC_SCRIPTS mac/Scripts/*)
  set_source_files_properties(
    ${APP_MAC_SCRIPTS}
    PROPERTIES
    MACOSX_PACKAGE_LOCATION Resources/Scripts
  )
  set(APP_MAC_FILES ${APP_MAC_RESOURCES} ${APP_MAC_SCRIPTS})
endif (APPLE)

qt4_wrap_ui(APP_UIS_H ${APP_UIS})
qt4_add_resources(APP_RESOURCES ${APP_QRCS})
qt4_automoc(${APP_SOURCES})

add_executable(${CMAKE_PROJECT_NAME} WIN32 MACOSX_BUNDLE
  ${APP_UIS_H}
  ${APP_SOURCES}
  ${APP_RESOURCES}
  ${APP_MAC_FILES}
)
target_link_libraries(${CMAKE_PROJECT_NAME}
  ${QT_LIBRARIES}
  ${QT_QTNETWORK_LIBRARIES}
)

if (APPLE)
  set_target_properties(${CMAKE_PROJECT_NAME}
    PROPERTIES
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/mac/Info.plist
  )
  add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/mac/qtbundle ${CMAKE_PROJECT_NAME}.app
  )
endif (APPLE)
