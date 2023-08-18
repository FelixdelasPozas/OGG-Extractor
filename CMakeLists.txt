project(OGGExtractor)

cmake_minimum_required (VERSION 3.0.0)
cmake_policy(SET CMP0053 NEW)
cmake_policy(SET CMP0071 OLD)

# Version Number
set (OGG_EXTRACTOR_VERSION_MAJOR 1)
set (OGG_EXTRACTOR_VERSION_MINOR 8)
set (OGG_EXTRACTOR_VERSION_PATCH 0)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

# Find the QtWidgets library
find_package(Qt5 COMPONENTS Widgets Multimedia)

if(WIN32)
	find_package(Qt5 COMPONENTS WinExtras)
endif()

#libvorbis
if(WIN32)
set(LIBVORBIS_INCLUDE_DIR "D:/Desarrollo/Bibliotecas/libvorbis-1.3.5/include")
set(LIBVORBIS_LIBRARY_DIR "D:/Desarrollo/Bibliotecas/libvorbis-1.3.5/lib/.libs")
set(LIBVORBIS_LIBRARIES   ${LIBVORBIS_LIBRARY_DIR}/libvorbisfile.a
                          ${LIBVORBIS_LIBRARY_DIR}/libvorbis.a)
else()
find_library(LIBVORBISFILE_LIBRARY vorbisfile libvorbisfile libvorbisfile-3 )
find_library(LIBVORBIS_LIBRARIES libvorbis )
endif()

#libogg
if(WIN32)
set(LIBOGG_INCLUDE_DIR "D:/Desarrollo/Bibliotecas/libogg-1.3.2/include")
set(LIBOGG_LIBRARY "D:/Desarrollo/Bibliotecas/libogg-1.3.2/src/.libs/libogg.a")
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
  ${LIBOGG_INCLUDE_DIR}
  )

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add Qt Resource files
qt5_add_resources(RESOURCES
  rsc/resources.qrc
)

qt5_wrap_ui(CORE_UI
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
  OGGExtractor.cpp
  AboutDialog.cpp
  OGGContainerWrapper.cpp
  ScanThread.cpp
)

set (CLI_SOURCES
  main-cli.cpp
  OGGContainerWrapper.cpp
)

set(OGG_LIBS
  ${LIBVORBIS_LIBRARIES}
  ${LIBOGG_LIBRARY}
  ${LIBOGG_LIBRARIES}
)

set(EXTERNAL_LIBS
  Qt5::Widgets
  Qt5::Multimedia
  ${OGG_LIBS}
)

if(WIN32)
	set(EXTERNAL_LIBS ${EXTERNAL_LIBS} Qt5::WinExtras)
endif()

add_executable(OGGExtractor ${WINAPP_SOURCES})
target_link_libraries (OGGExtractor ${EXTERNAL_LIBS})
if(WIN32)
  set_target_properties(OGGExtractor PROPERTIES COMPILE_FLAGS "-mwindows ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS} ${Qt5Multimedia_EXECUTABLE_COMPILE_FLAGS}" )
  target_link_options(OGGExtractor PUBLIC "-mwindows")
endif()
qt5_use_modules(OGGExtractor Widgets Multimedia)

add_executable(OGGExtractor-cli ${CLI_SOURCES})
target_link_libraries (OGGExtractor-cli ${OGG_LIBS})