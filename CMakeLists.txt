cmake_minimum_required (VERSION 3.10.0)

project(OGGExtractor)

cmake_policy(SET CMP0053 NEW)
cmake_policy(SET CMP0071 OLD)

# Version Number
set (OGG_EXTRACTOR_VERSION_MAJOR 1)
set (OGG_EXTRACTOR_VERSION_MINOR 11)
set (OGG_EXTRACTOR_VERSION_PATCH 1)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

string(TIMESTAMP CURRENT_YEAR "%Y")

# Find the QtWidgets library
find_package(Qt6 COMPONENTS Widgets Multimedia)

#libvorbis
if(WIN32)
set(LIBVORBIS_INCLUDE_DIR "D:/Desarrollo/Bibliotecas/libvorbis-1.3.7/include")
set(LIBVORBIS_LIBRARY_DIR "D:/Desarrollo/Bibliotecas/libvorbis-1.3.7/build/lib/")
set(LIBVORBIS_LIBRARIES   ${LIBVORBIS_LIBRARY_DIR}/libvorbisfile.a
                          ${LIBVORBIS_LIBRARY_DIR}/libvorbis.a)
else()
find_library(LIBVORBISFILE_LIBRARY vorbisfile libvorbisfile libvorbisfile-3 )
find_library(LIBVORBIS_LIBRARIES libvorbis )
endif()

#libogg
if(WIN32)
set(LIBOGG_INCLUDE_DIRS "D:/Desarrollo/Bibliotecas/libogg-1.3.5/include" "D:/Desarrollo/Bibliotecas/libogg-1.3.5/build/include")
set(LIBOGG_LIBRARY "D:/Desarrollo/Bibliotecas/libogg-1.3.5/build/libogg.a")
else()
find_library(LIBOGG_LIBRARY libogg)
endif()

if (CMAKE_BUILD_TYPE MATCHES Debug)
  set(EXTERNAL_LIBS ${EXTERNAL_LIBS} ${QT_QTTEST_LIBRARY})
endif (CMAKE_BUILD_TYPE MATCHES Debug)

if(DEFINED MINGW)
  configure_file("${PROJECT_SOURCE_DIR}/resources.rc.in" "${PROJECT_BINARY_DIR}/resources.rc")
  configure_file("${PROJECT_SOURCE_DIR}/installer/script.iss.in" "${PROJECT_BINARY_DIR}/script.iss")
  set(WINAPP_SOURCES ${WINAPP_SOURCES} ${CMAKE_CURRENT_BINARY_DIR}/resources.rc)
  set(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> -O coff -o <OBJECT> -i <SOURCE>")
  enable_language(RC)
endif(DEFINED MINGW)

include_directories(
  ${CMAKE_SOURCE_DIR}
  ${PROJECT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}          # Generated .h files
  ${CMAKE_CURRENT_BINARY_DIR}  # For wrap/ui files
  ${LIBVORBIS_INCLUDE_DIR}
  ${LIBOGG_INCLUDE_DIRS}
  )

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add Qt Resource files
qt6_add_resources(RESOURCES
  rsc/resources.qrc
)

qt6_wrap_ui(CORE_UI
  # .ui for Qt
  AboutDialog.ui
  OGGExtractor.ui
)
	
set (WINAPP_SOURCES
  # project files
  ${WINAPP_SOURCES}
  ${RESOURCES}
  ${CORE_MOC}
  ${CORE_UI}
  main.cpp
  TableModel.cpp
  OGGExtractor.cpp
  AboutDialog.cpp
  OGGContainerWrapper.cpp
  ScanThread.cpp
  Utils.cpp
  external/QTaskBarButton.cpp
)

set (CLI_SOURCES
  main-cli.cpp
  OGGContainerWrapper.cpp
)

set(OGG_LIBS
  ${LIBVORBIS_LIBRARIES}
  ${LIBOGG_LIBRARY}
)

set(EXTERNAL_LIBS
  Qt6::Widgets
  Qt6::Multimedia
  ${OGG_LIBS}
)

add_executable(OGGExtractor ${WINAPP_SOURCES})
target_link_libraries (OGGExtractor ${EXTERNAL_LIBS})
if(WIN32)
  set_target_properties(OGGExtractor PROPERTIES COMPILE_FLAGS "-std=c++20 -m64 -Wall -mwindows -Wno-deprecated ${Qt6Widgets_EXECUTABLE_COMPILE_FLAGS} ${Qt6Multimedia_EXECUTABLE_COMPILE_FLAGS}" )
  target_link_options(OGGExtractor PUBLIC "-mwindows")
endif()

add_executable(OGGExtractor-cli ${CLI_SOURCES})
target_link_libraries (OGGExtractor-cli ${OGG_LIBS})