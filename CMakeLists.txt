cmake_minimum_required(VERSION "2.6.0")
PROJECT(cockpit)

set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "1")
set(CPACK_PACKAGE_VERSION_PATCH "0")

SET(COCKPIT_VERSION ${CPACK_PACKAGE_VERSION_MAJOR})
SET(PROJECT_NAME cockpit)

if(WIN32)
  set(GLIB2_DIR     /usr/src/races/robot/src/ExtLibs/glib-2.0)
  set(GSTREAMER_DIR /usr/src/races/robot/src/ExtLibs/gstreamer-0.10)
  set(LIBXML2_DIR /usr/src/races/robot/src/ExtLibs/libxml2)
  list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake-modules/windows)
else(WIN32)
  list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake-modules/linux)
endif (WIN32)

find_package(Glib2 REQUIRED)
find_package(Gstreamer REQUIRED)
find_package(Ice REQUIRED)
find_package(SDL REQUIRED)
#find_package(SDL_image REQUIRED)
find_package(OpenGL REQUIRED)
find_package(OpenCV REQUIRED)
find_package(CURL REQUIRED)
set(Boost_USE_STATIC_LIBS   ON)
set(Boost_USE_MULTITHREADED ON)
find_package(Boost REQUIRED COMPONENTS filesystem system)

add_subdirectory(daeview)

add_definitions(-DGLEW_STATIC)

include_directories(${GLIB2_INCLUDE_DIR} 
                    ${GSTREAMER_INCLUDE_DIR}
                    ${LIBXML2_INCLUDE_DIR}
                    ${SDL_INCLUDE_DIR}
                    ${SDLIMAGE_INCLUDE_DIR}
                    ${ICE_INCLUDE_DIR}
                    ${PROJECT_BINARY_DIR}
                    ${OpenCV_INCLUDE_DIR}
                    ${CURL_INCLUDE_DIRS}
                    ${daeview_SOURCE_DIR}/include/collada-dom 
                    ${daeview_SOURCE_DIR}/include/collada-dom/1.4
                    ${daeview_SOURCE_DIR}/include
                    ${bullet-physics_SOURCE_DIR}/src
                    ${Boost_INCLUDE_DIRS})

link_directories(${GLIB2_LIBRARIES}
                 ${GSTREAMER_LIBRARIES}
                 ${ICE_LIBRARY_DIR}
                 ${collada-view_BINARY_DIR})

set(ICE_SLICE2CPP slice2cpp)

add_custom_command(OUTPUT ${PROJECT_BINARY_DIR}/admin.cpp ${PROJECT_BINARY_DIR}/admin.h
                   COMMAND ${ICE_SLICE2CPP} -I${ICE_SLICE_DIR} --output-dir ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}/remote-interfaces/admin.ice
                   MAIN_DEPENDENCY ${PROJECT_SOURCE_DIR}/remote-interfaces/admin.ice
                   COMMENT "invoking slice2cpp on ${PROJECT_SOURCE_DIR}/remote-interfaces/admin.ice")

add_custom_command(OUTPUT ${PROJECT_BINARY_DIR}/sensors.cpp ${PROJECT_BINARY_DIR}/sensors.h
                   COMMAND ${ICE_SLICE2CPP} -I${ICE_SLICE_DIR} --output-dir ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}/remote-interfaces/sensors.ice
                   MAIN_DEPENDENCY ${PROJECT_SOURCE_DIR}/remote-interfaces/sensors.ice
                   COMMENT "invoking slice2cpp on ${PROJECT_SOURCE_DIR}/remote-interfaces/sensors.ice")

add_custom_command(OUTPUT ${PROJECT_BINARY_DIR}/actuators.cpp ${PROJECT_BINARY_DIR}/actuators.h
                   COMMAND ${ICE_SLICE2CPP} -I${ICE_SLICE_DIR} --output-dir ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}/remote-interfaces/actuators.ice
                   MAIN_DEPENDENCY ${PROJECT_SOURCE_DIR}/remote-interfaces/actuators.ice
                   COMMENT "invoking slice2cpp on ${PROJECT_SOURCE_DIR}/remote-interfaces/actuators.ice")

if(NOT WIN32)
  SET(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} " -Wall")
endif(NOT WIN32)


SET(CPPFILES 
    ${PROJECT_BINARY_DIR}/admin.cpp
    ${PROJECT_BINARY_DIR}/sensors.cpp
    ${PROJECT_BINARY_DIR}/actuators.cpp
    src/AnimationController.cpp
    src/toanaglyph.cpp
    src/TxtAreaPainter.cpp
    src/VideoPainter.cpp
    src/PrintStatusMsg.cpp
    src/SetStatusMsgCmd.cpp
    src/AnimationCmd.cpp
    src/RouterHelper.cpp
    src/VideoDecoder.cpp
    src/KeyboardAdminI.cpp
    src/KeyboardSensorI.cpp
    src/ActuatorController.cpp
    src/JoystickChassisCtl.cpp
    src/JoystickHeadCtl.cpp
    src/KeyboardChassisCtl.cpp
    src/KeyboardHeadCtl.cpp
    src/SensorFrameReceiverI.cpp
    src/VisualizationManager.cpp
    src/IceApp.cpp
    src/BufferQueue.cpp
    src/SensorFrameReceiverI.cpp
    src/MapDownloadThread.cpp
    src/TileManager.cpp
    src/SDLThread.cpp
    src/main.cpp)

SET(HEADERS
    ${PROJECT_BINARY_DIR}/admin.cpp
    ${PROJECT_BINARY_DIR}/sensors.cpp
    ${PROJECT_BINARY_DIR}/actuators.cpp
    src/RangeMap.h
    src/AbstractCommand.h
    src/AnimationController.h
    src/BufferQueue.h
    src/KeyboardAdminI.h
    src/KeyboardSensorI.h
    src/ActuatorController.h
    src/JoystickChassisCtl.h
    src/JoystickHeadCtl.h
    src/KeyboardChassisCtl.h
    src/KeyboardHeadCtl.h
    src/SensorFrameReceiverI.h
    src/VisualizationManager.h
    src/IceApp.h
    src/PrintStatusMsg.h
    src/Queue.h
    src/RouterHelper.h
    src/VideoDecoder.h
    src/SensorFrameReceiverI.h
    src/SetStatusMsgCmd.h
    src/AnimationCmd.h
    src/toanaglyph.h
    src/TxtAreaPainter.h
    src/VideoPainter.h
    src/rect.h
    src/MapDownloadThread.h
    src/TileManager.h
    src/TileDownloadCmd.h
    src/SDLThread.h
    src/TileDownloadCmd.h)

SET(LIBRARIES
    collada-view
    ${OPENGL_LIBRARIES}
    ${SDL_LIBRARY}
    ${GLIB_glib_2_LIBRARY}
    ${GLIB_gobject_2_LIBRARY}
    ${GSTREAMER_gstreamer_LIBRARY}
    ${OpenCV_LIBS}
    ${CURL_LIBRARIES})

if(WIN32)
  add_executable(cockpit 
                 WIN32
                 ${CPPFILES}
                 ${HEADERS})
  target_link_libraries(cockpit
                        ${LIBRARIES}
                        debug Iced debug IceUtild debug Glacier2d
                        optimized Ice optimized IceUtil optimized Glacier2)
  INSTALL(FILES 
            ${PROJECT_BINARY_DIR}/vcredist_x86.exe 
            ${PROJECT_BINARY_DIR}/GStreamer-WinBuilds-GPL-x86.msi
          DESTINATION ./)
else(WIN32)
  add_executable(cockpit
                 ${CPPFILES}
                 ${HEADERS})
  target_link_libraries(cockpit
                        ${LIBRARIES}
                        ${Boost_LIBRARIES}
                        Ice IceUtil Glacier2)
  SET(CPACK_GENERATOR "DEB")
  SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "Andrey Nechypurenko") #required
endif(WIN32) 

INSTALL(TARGETS cockpit DESTINATION bin)
# INSTALL(FILES ${PROJECT_SOURCE_DIR}/ DESTINATION bin)

add_dependencies(cockpit collada-view)

#CPack configuration section

INCLUDE(InstallRequiredSystemLibraries)

SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Veter project")
SET(CPACK_PACKAGE_VENDOR "Andrey Nechypurenko")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")

SET(CPACK_PACKAGE_INSTALL_DIRECTORY "${PROJECT_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}")
IF(WIN32 AND NOT UNIX)
  SET(CPACK_NSIS_EXTRA_INSTALL_COMMANDS ${CPACK_NSIS_EXTRA_INSTALL_COMMANDS} "
      ExecWait 'msiexec.exe /qf /i \\\"$INSTDIR\\\\GStreamer-WinBuilds-GPL-x86.msi\\\"'
      ExecWait '\\\"$INSTDIR\\\\vcredist_x86.exe\\\" /q:a'
     ")

  # There is a bug in NSI that does not handle full unix paths properly. Make
  # sure there is at least one set of four (4) backlasshes.
#  SET(CPACK_PACKAGE_ICON "${CMake_SOURCE_DIR}/Utilities/Release\\\\InstallIcon.bmp")
  SET(CPACK_NSIS_INSTALLED_ICON_NAME "cockpit\\\\cockpit.exe")
  SET(CPACK_NSIS_DISPLAY_NAME "Veter Project")
  SET(CPACK_NSIS_HELP_LINK "https:\\\\\\\\www.gitorious.org\\\\veter\\\\pages\\\\Home")
  SET(CPACK_NSIS_URL_INFO_ABOUT "https:\\\\\\\\www.gitorious.org\\\\veter\\\\pages\\\\Home")
  SET(CPACK_NSIS_CONTACT "andreynech@gmail.com")
  SET(CPACK_NSIS_MODIFY_PATH OFF)
ELSE(WIN32 AND NOT UNIX)
  SET(CPACK_STRIP_FILES "bin/cockpit")
  SET(CPACK_SOURCE_STRIP_FILES "")
ENDIF(WIN32 AND NOT UNIX)
SET(CPACK_PACKAGE_EXECUTABLES "cockpit" "Cockpit")
INCLUDE(CPack)
